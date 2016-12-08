#include "coroutine_tool.h"

#include <boost/make_shared.hpp>

class test_class :public coroutine_tool::coroutine_env < std::tuple<int, int> >
{
public:
	virtual void entrance(yield_type &yield)
	{
		auto &tup = yield.get();

		printf("%d, %d\n", std::get<1>(tup), std::get<2>(tup));

		passCondition([](yield_type &yield){
			return true;
		});

		

		std::vector<spawn_func_type> vec;
		vec.push_back(std::bind(&test_class::branch1, this, std::placeholders::_1));
		vec.push_back(std::bind(&test_class::branch2, this, std::placeholders::_1));
		spawn(vec);
		printf("end spawn !");

		yield();

		asyncSleep(dk_seconds(2));

		printf("end asyncSleep !!!\n");
	}

	void branch1(yield_type &yield)
	{
		printf("branch1 1!\n");
		yield();
		printf("branch1 2!\n");
		yield();
		printf("branch1 3!\n");
		yield();
		printf("branch1 4!\n");
		yield();
		printf("branch1 5!\n");
		yield();
		printf("branch1 6!\n");
	}

	void branch2(yield_type &yield)
	{
		printf("branch2 1!\n");
		yield();
		printf("branch2 2!\n");
		yield();
		printf("branch2 3!\n");
		yield();
		printf("branch2 4!\n");
		yield();
		printf("branch2 5!\n");
		yield();
		printf("branch2 6!\n");
	}
};





struct op
{

};

void as(int a, int b)
{
	test_class t;
	coroutine_tool::env_title title;
	t.call(title, a, b);
}

void testcompile()
{
	test_class t;

	coroutine_tool::env_title title;
	t.call(title, 1, 2);
	int a = 1, b = 2;
	t.call(title, a, b);
	

	as(1,2);
}

void test()
{
	boost::asio::io_service ioservice;

	auto t = boost::make_shared<test_class>();
	t->setIoService(ioservice);
	
	test_class::params_type tup;
	coroutine_tool::env_title title;
	t->call_with_tuple(tup);
	t->call_with_title(title);
	printf("call 1\n");
	t->call_with_title(title);
	printf("call 2\n");
	t->call_with_title(title);
	printf("call 3\n");
	t->call_with_title(title);
	printf("call 4\n");
	t->call_with_title(title);
	printf("call 5\n");
	t->call_with_title(title);
	printf("call 6\n");
	t->call_with_title(title);
	printf("call 7\n");
	t->call_with_title(title);
	printf("call 8\n");
	//boost::asio::io_service::work work(ioservice);
	ioservice.run();
}