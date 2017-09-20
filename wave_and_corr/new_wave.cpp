// new_wave.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"

#include "wavelet.h"
#include<fstream>
#include <iomanip>
#include <iostream>
#include <algorithm>

#define  FILTERLEN 16  //sym8���˲���ϵ������
#define  COFE_LENGTH 158 //�ֽ�����ĵ�Ƶ����Ƶϵ�����ȶ�Ϊ158
#define DWT_STAGE	 4
using namespace std;


static const float sym8lowd_q15[] = {
	0.001890, -0.000303, -0.014952, 0.003809, 0.049137, -0.027219, -0.051946, 0.364442, 0.777186, 0.481360,-0.061273, -0.143294, 0.007607, 0.031695, -0.000542, -0.003382, };
static const float sym8highd_q15[] = {
	-0.003382, 0.000542, 0.031695, -0.007607, -0.143294, 0.061273, 0.481360, -0.777186, 0.364442, 0.051946,-0.027219, -0.049137, 0.003809, 0.014952, -0.000303, -0.001890, };
static const float sym8lowr_q15[] = {
	-0.003382, -0.000542, 0.031695, 0.007607, -0.143294, -0.061273, 0.481360, 0.777186, 0.364442, -0.051946,-0.027219, 0.049137, 0.003809, -0.014952, -0.000303, 0.001890, };
static const float sym8highr_q15[] = {
	-0.001890, -0.000303, 0.014952, 0.003809, -0.049137, -0.027219, 0.051946, 0.364442, -0.777186, 0.481360,0.061273, -0.143294, -0.007607, 0.031695, 0.000542, -0.003382, };



float dwt_filter(float *pdatain, int ndatain)
{
	//plowdataout[],phighdataout []�ĳ���158��4��ֽ�֮������ĸ���
	//�Գ����غ�ÿ��ϵ������(srcLen+filterlen-1)/2����68+41+28+21
	float *phighdataout = new float[COFE_LENGTH];
	float *plowdataout = new float[COFE_LENGTH];

	decompose(pdatain, ndatain, plowdataout, phighdataout);

	//�õ���һ���Ƶϵ���������ֵthr
	int Det1len = 68;
	float *pDet1 = new float[Det1len];
	for (int i = 0; i < Det1len; ++i)
	{
		pDet1[i] = phighdataout[i];
	}
	//float thr = wavelet_getThr(pDet1, Det1len);
	////�Ը�Ƶϵ��������ֵȥ��
	//wthresh(phighdataout, thr, COFE_LENGTH);

	//��ȥ���������С����任
	recompose(pdatain, ndatain, plowdataout, phighdataout);

	delete[]phighdataout;
	phighdataout = NULL;

	delete[]plowdataout;
	plowdataout = NULL;

	delete[]pDet1;
	pDet1 = NULL;
	return 1;
}


//���H1 H2 H3 H4, L1 L2 L3 L4
float decompose(float *pdatain, int ndatain, float *plowdataout, float *phighdataout)
{
	int nlevel, decLen;
	decLen = ndatain;

	for (nlevel = 0; nlevel<DWT_STAGE; nlevel++)
	{
		decLen = DWT(pdatain, decLen, plowdataout, phighdataout); //����DWT���������طֽ�ϵ���ĳ���
		pdatain = plowdataout;      //��Ƶ��������Ϊ���룬�����ֽ�       
		plowdataout += decLen;      //��һ������������һ�㣬���õ�һ�����飬Ҳ���Էֿ�����������
		phighdataout += decLen;
	}
	return 1;
}


/**
 * @brief				����ع�����
 * @param pdataout		�������
 * @param ndataout		�������
 * @param pLowDatain	��Ƶ�ֽ�ϵ��
 * @param pHighDatain	��Ƶ�ֽ�ϵ��
 * �򵥹��̣�
 * ����H1 H2 H3 H4, L1 L2 L3 L4��ʹ����H1 H2 H3 H4,  L4
 * L4+H4->L3, L3+H3->L2, L2+H2-> L1 , ���L1+H1 -> out
 **/
float recompose(float *pdataout, int ndataout, float *pLowDatain, float *pHighDatain)
{
	int nlevel, cALength;
	float *pLow = new float[68]; //pLow��ൽ68,��һ�����
	float*pHigh = NULL;
	static int StageBack[] = { 68,41,28,21 };  //�Գ����غ�ÿ��ϵ������(srcLen+filterlen-1)/2

	//pLow = pLowDatain + StageBack[0] ; //�Ƚ�pLow��pHigh��λ�����һ��
	int init_plow = COFE_LENGTH - StageBack[3];
	for (int i = 0; i < StageBack[3]; ++i)
	{
		pLow[i] = pLowDatain[init_plow + i];
	}
	pHigh = pHighDatain + StageBack[0] + StageBack[1] + StageBack[2];
	for (nlevel = DWT_STAGE; nlevel >= 1; nlevel--)
	{
		cALength = StageBack[nlevel - 1];
		Idwt(pLow, pHigh, cALength, pdataout);
		if (nlevel > 1)
		{
			for (int i = 0; i < StageBack[nlevel - 2]; ++i) //pLow���µ���һ��
			{
				pLow[i] = pdataout[i];
			}
			//pLow = pdataout; //�˴�����ֱ�Ӹ�ָ�룬����pdataoutֵ�ı䣬��pLowֵҲ�����ı䣬��Ӱ��������ؼ���
			pHigh -= StageBack[nlevel - 2]; //��Ƶϵ��ֱ��������һ��
		}		
	}
	delete[]pLow;
	pLow = NULL;
	return 1;
}


/**
* @brief 			С���任֮�ֽ�
* @param pSrcData 	�ֽ��Դ�ź�
* @param srcLen 	Դ�źŵĳ���
* @param cA 		�ֽ��Ľ��Ʋ�������-��Ƶ����
* @param cD 		�ֽ���ϸ�ڲ�������-��Ƶ����
*/

int  DWT(float *pSrcData, int srcLen, float *cA, float *cD)
{
	//��ֹ��������������������ݳ����Գ����ӳ���ΪfilterLen-1�����������signalLen��Խ�磩

	//if (srcLen < m_dbFilter.filterLen - 1)    //filterLenΪ��ͨ��ͨ�˲�ϵ������
	if (srcLen < FILTERLEN - 1)
	{
		cout << "������Ϣ���˲������ȴ����ź�!" << endl;
		exit(1);
	}
	int exLen = (srcLen + FILTERLEN - 1) / 2;//�Գ����Ӻ�ϵ���ĳ���
	int k = 0;
	float tmp = 0.0;
	for (int i = 0; i < exLen; i++)
	{

		cA[i] = 0.0;
		cD[i] = 0.0;
		for (int j = 0; j < FILTERLEN; j++)
		{
			k = 2 * i - j + 1;
			//�źű��ضԳ�����
			if ((k<0) && (k >= -FILTERLEN + 1))//���������
				tmp = pSrcData[-k - 1];
			else if ((k >= 0) && (k <= srcLen - 1))//���ֲ���
				tmp = pSrcData[k];
			else if ((k>srcLen - 1) && (k <= (srcLen + FILTERLEN - 2)))//�ұ�������
				tmp = pSrcData[2 * srcLen - k - 1];
			else
				tmp = 0.0;
			cA[i] += sym8lowd_q15[j] * tmp;
			cD[i] += sym8highd_q15[j] * tmp;
		}
	}
	return  exLen;  //���طֽ��ϵ���ĳ���
}


/**
* @brief 			С���任֮�ع�
* @param cA 		�ֽ��Ľ��Ʋ�������-��Ƶ����
* @param cD 		�ֽ���ϸ�ڲ�������-��Ƶ����
* @param cALength 	�����ֽ�ϵ�������ݳ���
* @param recData 	�ع������������
*/
void  Idwt(float *cA, float *cD, int cALength, float *recData)
{
	if ((NULL == cA) || (NULL == cD) || (NULL == recData))
		return;

	int n, k, p, recLen;

	//����121���Ĳ�ֽ�68,41,28,21��ż����cALength�ع�ʱ����һ����
	if(cALength % 2  != 0)
		 recLen = 2 * cALength - FILTERLEN + 2;  
	else
		recLen = 2 * cALength - FILTERLEN + 1;
	//cout << "recLen=" << recLen << endl;

	for (n = 0; n<recLen; n++)
	{
		recData[n] = 0;
		for (k = 0; k<cALength; k++)
		{
			p = n - 2 * k + FILTERLEN - 2;

			//�ź��ع�
			if ((p >= 0) && (p<FILTERLEN))
			{
				recData[n] += sym8lowr_q15[p] * cA[k] + sym8highr_q15[p] * cD[k];
				//cout<<"recData["<<n<<"]="<<recData[n]<<endl;
			}

		}
	}
}


/**
 * @brief			����ϸ��ϵ�������źų��ȼ�����ֵ
 * @param  pDetCoef ��һ��ϸ��ϵ��
 * @param  detlen   �䳤��
 **/
float wavelet_getThr(float *pDetCoef, int detlen)
{
	float thr = 0.0;
	float sigma = 0.0;
	for (int i = 0; i < detlen; ++i)
	{
		pDetCoef[i] = abs(pDetCoef[i]);
	}

	std::sort(pDetCoef, pDetCoef + detlen); //��ϸ��ϵ������������ȡ��ֵ�����׼ƫ��
	if (detlen % 2 == 0 && detlen >= 2)
	{
		sigma = (pDetCoef[detlen / 2 - 1] + pDetCoef[detlen / 2]) / 2 / 0.6745;
	}

	else
		sigma = pDetCoef[detlen / 2] / 0.6745;
	float N = INPUT_LEN; 		//�����źų���
	thr = sigma*sqrt(2.0*log(N)); //�����ֵ

	return thr;

}

 
/**
* @brief				��Ƶϵ����ֵȥ�뺯��
* @param  pDstCoef		���и�Ƶϵ��
* @param  high_length   ��Ƶϵ���ܳ���
**/
void wthresh(float *pDstCoef, float thr, int high_length)
{
	for (int i = 0; i < high_length; ++i)
	{
		if (abs(pDstCoef[i] < thr)) //С����ֵ������
		{
			pDstCoef[i] = 0.0;
		}
		else 		//������ֵ����������ֵ����
		{
			if (pDstCoef[i] < 0.0)
				pDstCoef[i] = thr - abs(pDstCoef[i]);
			else
				pDstCoef[i] = abs(pDstCoef[i]) - thr;
		}
	}
}

