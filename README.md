# zipkin_ice
zipkin instrumented library for ice

### 介绍
---
zipkin_ice是用于[ice](https://zeroc.com/)的zipkin装备库。


### 功能
---


### 依赖
---
* cpprestsdk
* C++ 11

### 待支持
---


### 示例
---
```c++
#include "zipkin_ice.h"
using namespace zipkin_ice;
int main(int argc, char *argv[])
{
	IceContext ctx;
	Tracer client;
	client.url(U("http://127.0.0.1:9411")).localEndpoint("service1", "192.168.1.1");
	Tracer server;
	server.url(U("http://127.0.0.1:9411")).localEndpoint("service2", "192.168.1.2");
	Span s1 = client.newTrace();
	s1.name(L"fun1").kind(CLIENT).annotate(CLIENT_SEND).remoteEndpoint("service2", "192.168.1.2").start();
	for (int i = 0; i < 10; i++);
	client.inject(s1.context(), ctx);
	Span s2 = server.joinSpan(server.extract(ctx));
	s2.kind(SERVER).annotate(SERVER_RECV).start();
	for (int i = 0; i < 10; i++);
	s2.annotate(SERVER_SEND).finish();
	for (int i = 0; i < 10; i++);
	s1.annotate(CLIENT_RECV).finish();
	client.report();
	server.report();
	system("pause");
	return 0;
}
```




