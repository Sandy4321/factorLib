/*
 * factorUtil.h
 *
 *  Created on: Feb 15, 2015
 *      Author: nicodjimenez
 */

#ifndef FACTORUTIL_H_
#define FACTORUTIL_H_

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>

using namespace std;
class Factor{
public:
	Factor(double *arr, vector<int> card,  vector<int> stride, int arrSize, vector<int> indexArr);

	~Factor() {
		if (arr_ != NULL){
            delete[] arr_;
			arr_ = NULL;
		}
	}

	void print(){
		cout.precision(10);
		for (auto ind : indexArr_){
			cout << ind << " ";
		}
		cout << endl;
		for (int i = 0; i < arrSize(); i++){
			cout << val(i);
			cout << " ";
		}
		cout << endl;
	}

	void isNormalized(){
		double sum = 0;
		for (int i = 0; i < arrSize_; i++){
			sum += arr_[i];
		}
		for (int i = 0; i < arrSize_; i++){
			arr_[i] = arr_[i] / sum;
		}
	}

	static shared_ptr<Factor> instanceNew(double *arr, vector<int> card,  vector<int> stride, int arrSize, vector<int> indexArr){
		Factor *factor = new Factor(arr, card, stride, arrSize, indexArr);
		shared_ptr<Factor> Ptr(factor);
		return Ptr;
	}

	double val(int index){
		return arr_[index];
	}

	int arrSize(){
		return arrSize_;
	}

	double *arr(){
		return arr_;
	}

	const vector<int>& indexArr(){
		return indexArr_;
	}

	int card(int varIndex){
		// returns cardinality of variable corresponding to variable index
		const auto iter = find(indexArr_.begin(), indexArr_.end(), varIndex);
		if (iter == indexArr_.end()){
			return 0;
		}
		else {
            int index = distance(indexArr_.begin(), iter);
            return card_[index];
		}
	}

	int stride(int varIndex){
		const auto iter = find(indexArr_.begin(), indexArr_.end(), varIndex);
		if (iter == indexArr_.end()){
			return 0;
		}
		else {
            int index = distance(indexArr_.begin(), iter);
            return stride_[index];
		}
	}

	shared_ptr<Factor> factorMarginalized(vector<int> margIndexArr){
		// returns new factor that marginalizes the given indices
		vector<int> indexArrNew;
		vector<int> cardArrNew;
		int arrSizeNew = 1;
		for (const auto& x : indexArr_){
			if (find(margIndexArr.begin(), margIndexArr.end(), x) == margIndexArr.end()){
				indexArrNew.push_back(x);
				int curCard = card(x);
				cardArrNew.push_back(curCard);
				arrSizeNew *= curCard;
			}
		}
        int varCt = indexArrNew.size();

        // initialize new stride array
        vector<int> strideArrNew(varCt);
        strideArrNew[0] = 1;
        for (int i = 1; i < varCt; i++){
            strideArrNew[i] = strideArrNew[i-1] * cardArrNew[i-1];
        }

        // initialize assignments to variables in new factor
        int assignment[varCt];
        for (int i = 0; i < varCt; i++){
        	assignment[i] = 0;
        }

        // initialize assignments to variables we are doing marginals over
        int margVarCt = margIndexArr.size();
        int margAssignment[margVarCt];
        for (int i = 0; i < margVarCt; i++){
        	margAssignment[i] = 0;
        }
        int margArrSize = 1;
        for (const auto& margVar : margIndexArr){
        	int newCard = card(margVar);
        	margArrSize = margArrSize * newCard;
        }

        // initialize new array
        double *arrNew = new double[arrSizeNew]();
        int j = 0;
        for (int i = 0; i < arrSizeNew; i++){

        	// iterate over marginalized variables
        	for (int k = 0; k < margArrSize; k++) {
        		arrNew[i] += arr_[j];
        		// update assignment of marginalized variables
                for (int n = 0; n < margVarCt; n++){
                    int curMargIndex = margIndexArr[n];
                    margAssignment[n]++;
                    if (margAssignment[n] == card(curMargIndex)){
                        margAssignment[n] = 0;
                        j = j - (card(curMargIndex) - 1) * stride(curMargIndex);
                    }
                    else {
                        j = j + stride(curMargIndex);
                        break;
                    }
                }
        	}

        	// update variable in factor
            for (int l = 0; l < varCt; l++){
                int curIndex = indexArrNew[l];
                assignment[l]++;
                if (assignment[l] == cardArrNew[l]){
                    assignment[l] = 0;
                    j = j - (cardArrNew[l] - 1) * stride(curIndex);
                }
                else {
                    j = j + stride(curIndex);
                    break;
                }
            }
        }

        auto factorNew = Factor::instanceNew(arrNew, cardArrNew, strideArrNew, arrSizeNew, indexArrNew);
        return factorNew;
        }

private:
	//TODO: use unique_ptr
	double *arr_;
	vector<int> card_;
	vector<int> stride_;
	int arrSize_;

	// specifies scope of factor
	vector< int> indexArr_;
};

Factor::Factor(double *arr,  vector<int> card, vector<int> stride, int arrSize, vector< int> indexArr) :
		arr_(arr),
		card_(card),
		stride_(stride),
		arrSize_(arrSize),
		indexArr_(indexArr)
{
}

shared_ptr<Factor> factorProduct(shared_ptr<Factor> phi1, shared_ptr<Factor> phi2){
	// first compute number of variables in new factor
	vector <int> indexArrNew;
	vector <int> cardArrNew;
	int arrSizeNew = 1;
	int cardNew;
    for (const int &varIndex1 : phi1->indexArr()){
        indexArrNew.push_back(varIndex1);
        cardNew = phi1->card(varIndex1);
        arrSizeNew *= cardNew;
        cardArrNew.push_back(cardNew);
	}
    for (const int &varIndex2 : phi2->indexArr()){
		// if we can't find the index, add it
        if (find(indexArrNew.begin(), indexArrNew.end(), varIndex2) == indexArrNew.end()){
            indexArrNew.push_back(varIndex2);
            cardNew = phi2->card(varIndex2);
            arrSizeNew *= cardNew;
            cardArrNew.push_back(cardNew);
        }
	}
	int varCt = cardArrNew.size();

	// initialize new stride array
	vector<int> strideArrNew(varCt);
	strideArrNew[0] = 1;
	for (int i = 1; i < varCt; i++){
		strideArrNew[i] = strideArrNew[i-1] * cardArrNew[i-1];
	}

	// initialize assignments
	int assignment[varCt];
	for (int l = 0; l < varCt; l++){
		assignment[l] = 0;
	}

	// initialize new array
	double *arrNew = new double[arrSizeNew]();
	int j = 0;
	int k = 0;
	for (int i = 0; i < arrSizeNew; i++){
		arrNew[i] = phi1->val(j) * phi2->val(k);

		for (int l = 0; l < varCt; l++){
            int curIndex = indexArrNew[l];
			assignment[l]++;

			if (assignment[l] == cardArrNew[l]){
				assignment[l] = 0;
				j = j - (cardArrNew[l] - 1) * phi1->stride(curIndex);
				k = k - (cardArrNew[l] - 1) * phi2->stride(curIndex);
			}
			else {
				j = j + phi1->stride(curIndex);
				k = k + phi2->stride(curIndex);
				break;
			}
		}
	}

	auto factorNew = Factor::instanceNew(arrNew, cardArrNew, strideArrNew, arrSizeNew, indexArrNew);
	return factorNew;
}


shared_ptr<Factor> factorDivide(shared_ptr<Factor> phi1, shared_ptr<Factor> phi2){
	// does phi1/phi2
	// first compute number of variables in new factor
	vector <int> indexArrNew;
	vector <int> cardArrNew;
	int arrSizeNew = 1;
	int cardNew;
	for (const  int &varIndex1 : phi1->indexArr()){
        indexArrNew.push_back(varIndex1);
        cardNew = phi1->card(varIndex1);
        arrSizeNew *= cardNew;
        cardArrNew.push_back(cardNew);
	}
	for (const  int &varIndex2 : phi2->indexArr()){
		// if we can't find the index, add it
        if (find(indexArrNew.begin(), indexArrNew.end(), varIndex2) == indexArrNew.end()){
            indexArrNew.push_back(varIndex2);
            cardNew = phi2->card(varIndex2);
            arrSizeNew *= cardNew;
            cardArrNew.push_back(cardNew);
        }
	}
	int varCt = cardArrNew.size();

	// initialize new stride array
	vector<int> strideArrNew(varCt);
	strideArrNew[0] = 1;
	for (int i = 1; i < varCt; i++){
		strideArrNew[i] = strideArrNew[i-1] * cardArrNew[i-1];
	}

	// initialize assignments
	int assignment[varCt];
	for (int l = 0; l < varCt; l++){
		assignment[l] = 0;
	}

	// initialize new array
	double *arrNew = new double[arrSizeNew]();
	int j = 0;
	int k = 0;
	for (int i = 0; i < arrSizeNew; i++){
		// make sure we arent dividing by zero
		double divVal = phi2->val(k) * 1.0;
		if (divVal == 0){
			divVal = 1.0;
		}
		arrNew[i] = phi1->val(j) / divVal;

		for (int l = 0; l < varCt; l++){
            int curIndex = indexArrNew[l];
			assignment[l]++;

			if (assignment[l] == cardArrNew[l]){
				assignment[l] = 0;
				j = j - (cardArrNew[l] - 1) * phi1->stride(curIndex);
				k = k - (cardArrNew[l] - 1) * phi2->stride(curIndex);
			}
			else {
				j = j + phi1->stride(curIndex);
				k = k + phi2->stride(curIndex);
				break;
			}
		}
	}

    auto factorNew = Factor::instanceNew(arrNew, cardArrNew, strideArrNew, arrSizeNew, indexArrNew);
	return factorNew;
}

#endif /* FACTORUTIL_H_ */
