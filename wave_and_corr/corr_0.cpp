// corr_0.cpp : �������̨Ӧ�ó������ڵ㡣
//
//
//

#include "stdafx.h"
#include<iostream>
#include<math.h>
#include<fstream>
#include<iomanip>
#include<time.h>
#include "corr.h"
#include "wavelet.h"

using namespace std;

float lorenz[LORENZ_LEN];	//����ȫ�����飬ֻ�趨��һ��
int maxi_ref; //ȫ�ֱ���

/**
* �������������ֵ
* �������ֵ�±�maxi
**/
int max(float a[], int size)
{
	int maxi = 0;
	for (int i = 0; i < size; ++i)
	{
		if (abs(a[maxi]) < abs(a[i]))
			maxi = i;
	}
	return maxi; 
}

/**
* ��������
**/
float dot_product(const float *x, const float *y, int length)
{
	float sum = 0;
	for (int i = 0; i < length; ++i)
	{
		sum += x[i] * y[i];
	}

	return sum;
}
/**
* corr����������Ϊÿһ�е����飬���䳤��
* �ڲ�����DSPF_sp_convol_cn��غ������׼���������
* ���Ϊ��ֵ������
**/
float corr(float *corr_input, float *lorenz, short input_len)
{
	float corr_output[CORR_OUTPUT_LEN];

	int maxi1, maxi2;	  //�����������ֵ���±�ֵ


	//����dsp��غ������õ�output���
	DSPF_sp_convol_cn(corr_input, lorenz, corr_output, input_len, LORENZ_LEN, CORR_OUTPUT_LEN, maxi_ref);

	//�Ȱѽ���ŵ�txt��������������matlab����������
	// if (result.is_open())
	// {
	// for (int i = 0; i < CORR_OUTPUT_LEN; ++i)
	// {
	// result << fixed << setprecision(8) << corr_output[i] << endl;
	// }
	// }

	//������������������ֵmaxi1 �� maxi2
	maxi1 = max(corr_output, CORR_OUTPUT_LEN);

	 if(corr_output[maxi1 + 1] > corr_output[maxi1 - 1])
	 {
		maxi2 = maxi1 +1;
	 }
	 else 
	 {
		maxi2 = maxi1;
		maxi1 = maxi1 - 1;
	 }

	// cout << "The output maxi1 and maxi12 : " << maxi1 <<"  "  << maxi2 << endl;

	//������������ֵ����
	int T, T1;	
	float maxi,frac;
	float c0_T, c0_T1;

	//�ȶ�������źź�����źţ��õ������źŶ�Ӧ�ķ�ֵ������maxi
	maxi = maxi1;    

	//�ҵ������źŷ�ֵ��ο��źŷ�ֵ����ʱ��һ����λ��T
	T = maxi - (LORENZ_LEN - maxi_ref);
	T1 = T + 1;

	//Խ��Ļ���ֱ��ȡfrac = 0.5
	if (T < 0 || T + LORENZ_LEN > input_len)
		frac = 0.5;

	 //ûԽ��������ֵ
	else
	{
		c0_T = corr_output[maxi1];
		c0_T1 = corr_output[maxi2];

		//�������źų���Ҫ�Ȳο��źų�������Խ��,��������ȡ100�����Ѿ�Խ�磬��һ��ȡ��TΪ23��23+100 > 121
		float cT_T, cT_T1, cT1_T1, denom;
		float *interpolation_input_T, *interpolation_input_T1;
		interpolation_input_T = &corr_input[T];
		interpolation_input_T1 = &corr_input[T1];

		//������,����Ҫ��һ��
		/*for (int i = 0; i < LORENZ_LEN; ++i)
		{
			interpolation_input_T[i] = *(&corr_input[T + i]);
		}

		for (int i = 0; i < LORENZ_LEN; ++i)
		{
			interpolation_input_T1[i] = *(&corr_input[T1 + i]);
		}*/

		cT_T = dot_product(interpolation_input_T, interpolation_input_T, LORENZ_LEN);
		cT_T1 = dot_product(interpolation_input_T, interpolation_input_T1, LORENZ_LEN);
		cT1_T1 = dot_product(interpolation_input_T1, interpolation_input_T1, LORENZ_LEN);

		denom = c0_T1*(cT_T - cT_T1) + c0_T*(cT1_T1 - cT_T1);   //��ĸ
		//if (abs(denom) > 0.01)  �����ĸ>0.01����������ĳ��ȼ����ֵ����ֵ�������1�Ͷ�Ϊ0.5
		frac = (c0_T1*cT_T - c0_T*cT_T1) / denom;
		//frac����1��ȡ��С��0��ֵ��ʱ�������⣬��ֵ�㷨ʹ�Եģ��������������⣬���ܺ����бȽϴ���������һ����󼸸���Ī������ܴ�
		if (frac > 1 || frac < 0)
			frac = 0.5;
	}
	maxi = maxi + frac;
	return maxi;
}



/**
* fitting����: ����collection_rows ��ʾ�ɼ��˶����У� datacount_cols��ʾ�ж�����
* inputָ�����������Ϊһά���飬output����������,�ⲿ��������ʹ��output
* ���������ݵ�ÿһ���Ƚ���С���˲����ٽ���������㣬�����ֵ������
**/

 void fitting(int collection_rows, int datacount_cols, float *input, float *output)
{
	int space = 0;
	float **p = new float*[collection_rows];					//pָ���ά����
	float *corr_input = new float[collection_rows];
	//output = new float[datacount_cols];    //ע������ڴ������ͷţ�
	float maxi;

	//��׼�������͵�����������aΪϵ���� bΪ����߿�cΪ��ߵ�ĺ����꼴����Ƶ��
	float a = 0.1, b = 0.052, c = 0.06;
	//�����������������ݣ��ȵõ�������F[]ֵ����ΧΪ-0.06~0.18������0.002 
	float F[LORENZ_LEN];
	for (int i = 0; i < LORENZ_LEN; i++)
	{
		F[i] = 0 + 0.002*i;
	}
	//�����׼����������ֵ
	for (int i = 0; i < LORENZ_LEN; i++)
	{
		lorenz[i] = a*(pow((b*0.5), 2)) / ((pow((b*0.5), 2)) + (pow((F[i] - c), 2)));
	}
	maxi_ref = max(lorenz, LORENZ_LEN);


	//��ÿһ�г�ʼ��datacount_cols����
	for (int i = 0; i < collection_rows; ++i)
		p[i] = new float[datacount_cols];

	//������ľ���ֵ,��input����p[][]
	for (int i = 0; i < collection_rows; ++i)
	{
		for (int j = 0; j < datacount_cols; ++j)
		{
			p[i][j] = input[j + space] ;
		}
		space = space + datacount_cols;
	}

	//��p[][]��ÿһ�е�����غ������õ�ÿһ�еķ�ֵ������,��ӵ�output
	for (int j = 0; j < datacount_cols; ++j)
	{
		for (int i = 0; i < collection_rows; ++i)
		{
			corr_input[i] = p[i][j]; //��һ��������corr_input[]��Ϊ��ص�����
		}

	    dwt_filter(corr_input, collection_rows); //����С���˲��������Ƚ����˲�
		maxi = corr(corr_input, lorenz, collection_rows); //������غ������õ���ֵ������
		output[j] = maxi;        //�ѽ����ӵ�output	
	}

	for (int i = 0; i < collection_rows; ++i)
		delete[]p[i];
	
	delete[]p;
	delete[]corr_input;
}
