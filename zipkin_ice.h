#pragma once
#include <memory>
#include <string>
#include <map>
#include <cpprest/json.h>   
#include <boost/lockfree/stack.hpp>

#ifdef _WIN32
#ifndef ZIPKIN_ICE_EXPORT
#define ZIPKIN_ICE_API __declspec(dllimport)
#else
#define ZIPKIN_ICE_API __declspec(dllexport)
#endif
#else
#define ZIPKIN_ICE_API
#endif

using namespace web;
namespace zipkin_ice 
{
	//************** Span.kind **************
	const std::wstring CLIENT = L"CLIENT";
	const std::wstring SERVER = L"SERVER";
	const std::wstring PRODUCER = L"PRODUCER";
	const std::wstring CONSUMER = L"CONSUMER";

	//************** Annotation.value **************
	const std::wstring CLIENT_SEND = L"cs";
	const std::wstring CLIENT_RECV = L"cr";
	const std::wstring SERVER_SEND = L"ss";
	const std::wstring SERVER_RECV = L"sr";
	const std::wstring MESSAGE_SEND = L"ms";
	const std::wstring MESSAGE_RECV = L"mr";
	const std::wstring WIRE_SEND = L"ws";
	const std::wstring WIRE_RECV = L"wr";
	const std::wstring CLIENT_SEND_FRAGMENT = L"csf";
	const std::wstring CLIENT_RECV_FRAGMENT = L"crf";
	const std::wstring SERVER_SEND_FRAGMENT = L"ssf";
	const std::wstring SERVER_RECV_FRAGMENT = L"srf";

	//***** Tag.key ******
	const std::wstring HTTP_HOST = L"http.host";
	const std::wstring HTTP_METHOD = L"http.method";
	const std::wstring HTTP_PATH = L"http.path";
	const std::wstring HTTP_URL = L"http.url";
	const std::wstring HTTP_STATUS_CODE = L"http.status_code";
	const std::wstring HTTP_REQUEST_SIZE = L"http.request.size";
	const std::wstring HTTP_RESPONSE_SIZE = L"http.response.size";
	const std::wstring LOCAL_COMPONENT = L"lc";

	//***** Annotation.value or Tag.key ******
	const std::wstring ERRORSTR = L"error";
	const std::wstring CLIENT_ADDR = L"ca";
	const std::wstring SERVER_ADDR = L"sa";
	const std::wstring MESSAGE_ADDR = L"ma";

	typedef std::shared_ptr<json::value> JsonValuePtr;
	class ZIPKIN_ICE_API Endpoint
	{
	public:
		Endpoint(const std::string& serviceName, 
			const std::string& ipv4, 
			uint16_t port = 0);

		JsonValuePtr toJson();

	private:
		std::string _ipv4;
		uint16_t _port;
		std::string _serviceName;
		//optional binary ipv6
	};
	typedef std::shared_ptr<Endpoint> EndpointPtr;


	class ZIPKIN_ICE_API Annotation
	{
	public:
		Annotation(uint64_t timestamp, 
			const std::wstring &value);

		JsonValuePtr toJson();

	private:
		uint64_t _timestamp = 0;
		std::wstring _value;
	};

	typedef std::pair<std::wstring, std::wstring> Tag;

	class ZIPKIN_ICE_API TraceContext
	{
		friend class Tracer;
		friend class Span;
	public:
		TraceContext();

		TraceContext(uint64_t traceId, uint64_t parentId, uint64_t spanId);

		TraceContext& traceId(uint64_t traceId);

		TraceContext& parentId(uint64_t parentId);

		TraceContext& spanId(uint64_t spanId);

		operator bool() const;

		JsonValuePtr toJson() const;

	protected:
		uint64_t _traceId = 0;
		uint64_t _parentId = 0;
		uint64_t _id = 0;
	};

	class Tracer;
	class ZIPKIN_ICE_API Span
	{
	public:
		Span(const TraceContext &context, Tracer *tracer);

		Span& name(const std::wstring &name);

		Span& kind(const std::wstring &kind);

		Span& shared(bool shared);

		Span& remoteEndpoint(const std::string &serviceName, const std::string& ipv4, uint16_t port = 0);

		Span& annotate(const std::wstring &value);

		Span& tag(const std::wstring &key, const std::wstring &value);

		Span& start();

		Span& finish();

		Span& flush();

		const TraceContext &context() const;

		JsonValuePtr toJson();

	private:
		uint64_t now();

	private:
		std::wstring _name;
		TraceContext _context;
		std::wstring _kind;
		uint64_t _timestamp = 0;
		uint64_t _duration = 0;
		bool _debug = false;
		bool _shared = false;
		//optional i64 trace_id_high
		EndpointPtr _remoteEndpoint;
		std::vector<Annotation> _annotations;
		std::vector<Tag> _tags;
		Tracer* _tracer = nullptr;
	};


	typedef std::map<std::string, std::string> IceContext;
	class ZIPKIN_ICE_API Tracer
	{
	public:
		Tracer();

		Tracer& localEndpoint(const std::string& serviceName, 
			const std::string& ipv4, 
			uint16_t port = 0);

		Tracer& url(const std::wstring& url);

		static TraceContext extract(const IceContext &icectx);

		void inject(const TraceContext &context, IceContext &icectx);

		Span newTrace();

		Span newChild(const TraceContext &parent);

		Span joinSpan(const TraceContext &parent);

		void add(JsonValuePtr span);

		void report();

	private:
		TraceContext nextContext(const TraceContext *parent);

		uint64_t genId();

	private:
		EndpointPtr _localEndpoint;
		boost::lockfree::stack<JsonValuePtr> _cache;
		std::wstring _url;
	};
}
