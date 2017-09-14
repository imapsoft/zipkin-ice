#include "zipkin_ice.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <boost/locale.hpp>
#include <cpprest/http_client.h>
using namespace web::http::client;
using namespace web::http;
namespace zipkin_ice
{
	std::wstring to_hexstring(uint64_t value)
	{
		std::wstringstream ss;
		ss << std::hex << value;
		return ss.str();
	}

	uint64_t hexstring_to_int(const std::wstring& value)
	{
		uint64_t result;
		std::wstringstream ss(value);
		ss >> std::hex >> result;
		return result;
	}


	Endpoint::Endpoint(const std::string &serviceName, const std::string& ipv4, uint16_t port)
		: _serviceName(serviceName), _ipv4(ipv4), _port(port)
	{

	}

	JsonValuePtr Endpoint::toJson()
	{
		auto obj = std::make_shared<json::value>(json::value::object(true));
		(*obj)[L"serviceName"] = json::value::string(boost::locale::conv::to_utf<wchar_t>(_serviceName, "GBK"));
		(*obj)[L"ipv4"] = json::value::string(boost::locale::conv::to_utf<wchar_t>(_ipv4, "GBK"));
		(*obj)[L"port"] = json::value::number(_port);
		return obj;
	}


	Annotation::Annotation(uint64_t timestamp, const std::wstring &value)
		: _timestamp(timestamp), _value(value)
	{

	}

	JsonValuePtr Annotation::toJson()
	{
		auto obj = std::make_shared<json::value>(json::value::object(true));
		(*obj)[L"timestamp"] = json::value::number(_timestamp);
		(*obj)[L"value"] = json::value::string(_value);
		return obj;
	}



	TraceContext::TraceContext()
	{

	}

	TraceContext::TraceContext(uint64_t traceId, uint64_t parentId, uint64_t spanId)
		: _traceId(traceId), _parentId(parentId), _id(spanId)
	{

	}

	TraceContext& TraceContext::traceId(uint64_t traceId)
	{
		_traceId = traceId;
		return *this;
	}

	TraceContext& TraceContext::parentId(uint64_t parentId)
	{
		_parentId = parentId;
		return *this;
	}

	TraceContext& TraceContext::spanId(uint64_t spanId)
	{
		_id = spanId;
		return *this;
	}

	TraceContext::operator bool() const
	{
		return _traceId > 0 && _id > 0;
	}

	JsonValuePtr TraceContext::toJson() const
	{
		auto obj = std::make_shared<json::value>(json::value::object(true));
		(*obj)[L"traceId"] = json::value::string(to_hexstring(_traceId));
		(*obj)[L"parentId"] = json::value::string(to_hexstring(_parentId));
		(*obj)[L"id"] = json::value::string(to_hexstring(_id));
		return obj;
	}



	Span::Span(const TraceContext &context, Tracer *tracer)
	{
		_context = context;
		_tracer = tracer;
	}

	Span& Span::name(const std::wstring &name)
	{
		_name = name;
		return *this;
	}

	Span& Span::kind(const std::wstring &kind)
	{
		_kind = kind;
		return *this;
	}

	Span& Span::shared(bool shared)
	{
		_shared = shared;
		return *this;
	}

	Span& Span::remoteEndpoint(const std::string &serviceName, const std::string& ipv4, uint16_t port)
	{
		_remoteEndpoint = std::make_shared<Endpoint>(serviceName, ipv4, port);
		return *this;
	}

	Span& Span::annotate(const std::wstring &value)
	{
		_annotations.push_back(Annotation(now(), value));
		return *this;
	}

	Span& Span::tag(const std::wstring &key, const std::wstring &value)
	{
		_tags.push_back(std::make_pair(key, value));
		return *this;
	}

	Span& Span::start()
	{
		_timestamp = now();
		return *this;
	}

	Span& Span::finish()
	{
		_duration = now() - _timestamp;
		_tracer->add(toJson());
		return *this;
	}

	Span& Span::flush()
	{
		_tracer->add(toJson());
		return *this;
	}

	const TraceContext& Span::context() const
	{
		return _context;
	}

	JsonValuePtr Span::toJson()
	{
		auto obj = std::make_shared<json::value>(json::value::object(true));
		(*obj)[L"traceId"] = json::value::string(to_hexstring(_context._traceId));
		(*obj)[L"name"] = json::value::string(_name);
		if (_context._parentId > 0)
			(*obj)[L"parentId"] = json::value::string(to_hexstring(_context._parentId));
		(*obj)[L"id"] = json::value::string(to_hexstring(_context._id));
		(*obj)[L"kind"] = json::value::string(_kind);
		(*obj)[L"timestamp"] = json::value::number(_timestamp);
		(*obj)[L"duration"] = json::value::number(_duration);
		(*obj)[L"debug"] = json::value::boolean(_debug);
		(*obj)[L"shared"] = json::value::boolean(_shared);
		if (_remoteEndpoint)
		{
			(*obj)[L"remoteEndpoint"] = *_remoteEndpoint->toJson();
		}
		if (!_annotations.empty())
		{
			(*obj)[L"annotations"] = json::value::array(_annotations.size());
			for (size_t i = 0; i < _annotations.size(); i++)
			{
				(*obj)[L"annotations"][i] = *_annotations[i].toJson();
			}

		}
		if (!_tags.empty())
		{
			auto tags = json::value::object(true);
			for (auto tag : _tags)
			{
				tags[tag.first] = json::value::string(tag.second);
			}
			(*obj)[L"tags"] = tags;
		}
		return obj;
	}


	uint64_t Span::now()
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(
			std::chrono::system_clock::now().time_since_epoch()).count();
	}


	const std::string ZIPKIN_CONTEXT = "zipkin_context";
	static thread_local std::shared_ptr<std::mt19937_64> g_Rnd;

	Tracer::Tracer() : _cache(64)
	{

	}

	Tracer& Tracer::localEndpoint(const std::string& serviceName, const std::string& ipv4, uint16_t port)
	{
		_localEndpoint = std::make_shared<Endpoint>(serviceName, ipv4, port);
		return *this;
	}

	Tracer& Tracer::url(const std::wstring& url)
	{
		_url = url;
		return *this;
	}

	TraceContext Tracer::extract(const IceContext &icectx)
	{
		if (icectx.find(ZIPKIN_CONTEXT) == icectx.end())
		{
			return TraceContext();
		}
		std::error_code error;
		std::stringstream ss(icectx.at(ZIPKIN_CONTEXT));
		auto obj = json::value::parse(ss, error);
		if (error)
		{
			return TraceContext();
		}
		else
		{
			uint64_t traceId = hexstring_to_int(obj.has_field(L"traceId") ? obj[L"traceId"].as_string() : L"");
			uint64_t parentId = hexstring_to_int(obj.has_field(L"parentId") ? obj[L"parentId"].as_string() : L"");
			uint64_t spanId = hexstring_to_int(obj.has_field(L"id") ? obj[L"id"].as_string() : L"");
			if (traceId <= 0 || spanId <= 0)
			{
				return TraceContext();
			}
			return TraceContext(traceId, parentId, spanId);
		}
	}

	void Tracer::inject(const TraceContext &context, IceContext &icectx)
	{
		std::stringstream ss;
		(*context.toJson()).serialize(ss);
		icectx[ZIPKIN_CONTEXT] = ss.str();
	}

	Span Tracer::newTrace()
	{
		return Span(nextContext(nullptr), this);
	}

	Span Tracer::newChild(const TraceContext &parent)
	{
		return Span(nextContext(&parent), this);
	}

	Span Tracer::joinSpan(const TraceContext &parent)
	{
		return Span(parent, this).shared(true);
	}

	void Tracer::add(JsonValuePtr span)
	{
		_cache.push(span);
	}

	static boost::lockfree::stack<Concurrency::task<void>> tasks(64);
	void Tracer::report()
	{
		std::vector<json::value> spans;
		_cache.consume_all([&spans, this](JsonValuePtr v) -> void
		{
			if (_localEndpoint)
				(*v)[L"localEndpoint"] = *_localEndpoint->toJson();
			spans.push_back(*v);
		});
		if (!spans.empty())
		{
			auto arrs = json::value::array(spans);
			http_client client(_url);
			uri_builder builder(U("/api/v2/spans"));
			auto task = client.request(methods::POST, builder.to_string(),
				arrs.serialize(), L"application/json").then([arrs](http_response response)
			{
				std::wcout << arrs.serialize() << ",status:" << response.status_code() << std::endl;
			});

			tasks.push(task);
		}
	}

	void Tracer::wait()
	{
		tasks.consume_all([&](auto &task)
		{
			task.wait();
		});
	}


	TraceContext Tracer::nextContext(const TraceContext *parent)
	{
		uint64_t nextId = genId();
		if (parent)
		{
			return TraceContext(parent->_traceId, parent->_id, nextId);
		}
		return TraceContext(nextId, 0, nextId);
	}


	uint64_t Tracer::genId()
	{
		if (!g_Rnd)
		{
			g_Rnd.reset(new std::mt19937_64(
				(std::chrono::system_clock::now().time_since_epoch().count() << 32)
				+ std::random_device()()));
		}
		std::mt19937_64 &rnd = *g_Rnd.get();
		return rnd();
	}


}
