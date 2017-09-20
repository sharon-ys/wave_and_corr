
#include "stdafx.h"
#include<iostream>
#include<math.h>
#include<fstream>
#include <iomanip>
#include "wavelet.h"
#include "corr.h"
using namespace std;

int main()
{
	clock_t cstart, cend;
	ifstream input_file;
	input_file.open("mydata.txt");

	ofstream final_result;
	final_result.open("final_result.txt");


	int collection_rows = INPUT_LEN;
	int datacount_cols = DATA_COLS;
	float *gets = new float[collection_rows*datacount_cols];    //gets��һ��һά����
	float *output = new float[datacount_cols];


	//��ȡ�����ļ�����
	if (!input_file.is_open())
	{
		cout << "Error opening file" << endl;
		exit(1);
	}

	for (int i = 0; ; ++i)
	{
		input_file >> gets[i];
		if (input_file.eof())
			break;
	}

	cstart = clock();

	//����fitting����,ÿһ���˲���������
	fitting(collection_rows, datacount_cols, gets, output);
	//�Ȱѽ���ŵ�txt��
	if (final_result.is_open())
	{
		for (int i = 0; i < datacount_cols; ++i)
		{
			//ÿһ���±��0��ʼ������output[i]���±꼴ֱ��*0.002��������
			final_result << fixed << setprecision(8) << 10.8 + (0.002*output[i]) << endl;
			//final_result << fixed << setprecision(8) << output[i] << endl;
		}
	}


	delete[]gets;
	delete[]output;
	input_file.close();
	final_result.close();

	cend = clock();
	cout << "time is " << cend - cstart << endl;

	return 0;
}


