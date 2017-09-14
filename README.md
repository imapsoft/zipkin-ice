# zipkin-ice
zipkin instrumented library for ice

### 介绍
---
zipkin_ice是用于[ice](https://zeroc.com/)的zipkin装备库。


### 依赖
---
* cpprestsdk
* C++ 11

### 功能
---



### 待支持
---
* 采样

### 示例
---
```c++
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
Tracer::wait();
```




