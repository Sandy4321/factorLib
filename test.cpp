#include <cstring>
#include "factorUtil.h"

void test_basic_op(){
	// tests factor product and factor marginalization
	double *arr_0 = new double[4]();
	vector<int> card_0;
	vector<int> stride_0;
	vector<int> indArr_0;

	// define variables
	arr_0[0] = 0.2;
	arr_0[1] = 0.2;
	arr_0[2] = 0.2;
	arr_0[3] = 0.1;
    double *arr_1 = new double[4]();
    memcpy(arr_1, arr_0, sizeof(double) * 4);
    indArr_0.push_back(1);
	indArr_0.push_back(2);
	card_0.push_back(2);
	card_0.push_back(2);
	stride_0.push_back(1);
	stride_0.push_back(2);
    auto fact_0 = Factor::instanceNew(arr_0, card_0, stride_0, 4, indArr_0);
    auto fact_1 = Factor::instanceNew(arr_1, card_0, stride_0, 4, indArr_0);
    auto fact_2 = factorProduct(fact_0, fact_1);
	assert (abs(fact_2->val(3) - 0.01) < 0.0001);
	vector<int> margIndArr;
	margIndArr.push_back(1);
    auto fact_3 = fact_2->factorMarginalized(margIndArr);
	assert (abs(fact_3->val(0) - 0.08) < 0.0001);
    std::cout << "Test passed!" << std::endl;
}

int main(){
    test_basic_op();
}

