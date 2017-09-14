#include "zipkin_ice.h"
using namespace zipkin_ice;

class Service
{
public:
	Service(const std::string& serviceName,
		const std::string& ipv4,
		uint16_t port = 0) : _serviceName(serviceName), _ipv4(ipv4), _port(port)
	{
		trace.localEndpoint(serviceName, ipv4, port).url(U("http://127.0.0.1:9411"));
	}
	

public:
	std::string _serviceName;
	std::string _ipv4;
	uint16_t _port;
	Tracer trace;
};

class Service4 : public Service
{
public:
	Service4() : Service("service4", "192.168.1.4", 4444)
	{

	}
	void fun4(IceContext& ctx)
	{
		Span s7 = trace.joinSpan(trace.extract(ctx));
		s7.kind(SERVER).annotate(SERVER_RECV).start();
		s7.annotate(SERVER_SEND).finish();
		trace.report();
	}
};

class Service2 : public Service
{
public:
	Service2() : Service("service2", "192.168.1.2", 2222)
	{

	}

	void fun2(IceContext& ctx, Service4& service4)
	{
		Span s4 = trace.joinSpan(trace.extract(ctx));
		s4.kind(SERVER).annotate(SERVER_RECV).start();

		Span s6 = trace.newChild(s4.context());
		trace.inject(s6.context(), ctx);
		s6.name(L"fun4").kind(CLIENT).annotate(CLIENT_SEND)
			.remoteEndpoint(service4._serviceName, service4._ipv4, service4._port).start();
		service4.fun4(ctx);
		s6.annotate(CLIENT_RECV).finish();

		s4.annotate(SERVER_SEND).finish();
		trace.report();
	}
};

class Service3 : public Service
{
public:
	Service3() : Service("service3", "192.168.1.3", 3333)
	{

	}
	void fun3(IceContext& ctx)
	{
		Span s5 = trace.joinSpan(trace.extract(ctx));
		s5.kind(SERVER).annotate(SERVER_RECV).start();
		s5.annotate(SERVER_SEND).finish();
		trace.report();
	}
};

class Service1 : public Service
{
public:
	Service1() : Service("service1", "192.168.1.1", 1111)
	{
	}


	void fun1(Service2& service2, Service3& service3, Service4& service4)
	{
		IceContext ctx;

		Span s1 = trace.newTrace();
		s1.name(L"fun1").kind(SERVER).annotate(SERVER_RECV).start();

		Span s2 = trace.newChild(s1.context());
		trace.inject(s2.context(), ctx);
		s2.name(L"fun2").kind(CLIENT).annotate(CLIENT_SEND)
			.remoteEndpoint(service2._serviceName, service2._ipv4, service2._port).start();
		service2.fun2(ctx, service4);
		s2.annotate(CLIENT_RECV).finish();

		Span s3 = trace.newChild(s1.context());
		trace.inject(s3.context(), ctx);
		s3.name(L"fun3").kind(CLIENT).annotate(CLIENT_SEND)
			.remoteEndpoint(service3._serviceName, service3._ipv4, service3._port).start();
		service3.fun3(ctx);
		s3.annotate(CLIENT_RECV).finish();

		s1.annotate(SERVER_SEND).finish();
		trace.report();
	}
};


int main(int argc, char *argv[])
{
	Service1 s1;
	Service2 s2;
	Service3 s3;
	Service4 s4;
	s1.fun1(s2, s3, s4);
	Tracer::wait();
	return 0;
}