#include "coroutine_tool.h"

class test_class :public coroutine_env < std::tuple<int, int> >
{
public:
	virtual void entrance(yield_type &yield)
	{
		auto &tup = yield.get();

		printf("%d, %d\n", std::get<0>(tup), std::get<1>(tup));
	}
};







void test()
{
	test_class t;

	int a = 1, b = 2;
	t.call(a, b);
}