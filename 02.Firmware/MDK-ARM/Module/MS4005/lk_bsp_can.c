# include "lk_bsp_can.h"
# include "bsp_can.h"
# include "can.h"

CAN_TxHeaderTypeDef hCAN1_TxHeader; //CAN1������Ϣ
CAN_RxHeaderTypeDef hCAN1_RxHeader; //CAN1������Ϣ
CAN_FilterTypeDef hCAN1_Filter; //CAN1�˲���



/*******************************************************************************
* Function Name  : vApp_CAN_TxHeader_Init
* Description    : ��ʼ������֡ͷ���
* Input          : pHeader ����֡ͷָ��
                   StdId ��ʶ��
                   ExtId ��չ��ʶ��
                   IDE 0:��׼֡ 1:��չ֡
                   RTR 0:����֡ 1:Զ��֡
                   DLC ���ݳ���
* Output         : None
* Return         : None
****************************************************************************** */
void vApp_CAN_TxHeader_Init(CAN_TxHeaderTypeDef    * pHeader,
                                                        uint32_t                             StdId, 
                                                        uint32_t                             ExtId, 
                                                        uint32_t                             IDE, 
                                                        uint32_t                             RTR, 
                                                        uint32_t                             DLC)
{
    pHeader->StdId    = StdId;    //11λ     ��׼��ʶ��
    pHeader->ExtId    = ExtId;    //29λ     ��չ��ʶ��
    pHeader->IDE        = IDE;        //1λ        0:��׼֡ 1:��չ֡
    pHeader->RTR        = RTR;      //1λ   0:����֡ 1:Զ��֡
    pHeader->DLC        = DLC;        //4λ   ���͵����ݵĳ���
    pHeader->TransmitGlobalTime    =    ENABLE;
}

/*******************************************************************************
* Function Name  : vApp_CAN_Filter_Init
* Description    : ��ʼ���˲���
* Input          : pFilter �˲����������ʼ��ȫ��ֵ
                                     IdHigh,
                   IdLow,
                   MaskIdHigh,
                   MaskIdLow,
                   FIFOAssignment,
                   Bank,
                   Mode,
                   Scale,
                   Activation,
                   SlaveStartFilterBank
* Output         : None
* Return         : None
****************************************************************************** */
void vApp_CAN_Filter_Init(CAN_FilterTypeDef * pFilter,
                                                    uint32_t IdHigh,
                                                    uint32_t IdLow,
                                                    uint32_t MaskIdHigh,
                                                    uint32_t MaskIdLow,
                                                    uint32_t FIFOAssignment,
                                                    uint32_t Bank,
                                                    uint32_t Mode,
                                                    uint32_t Scale,
                                                    uint32_t Activation,
                                                    uint32_t SlaveStartFilterBank)
{
    pFilter->FilterIdHigh                 = 0;
    pFilter->FilterIdLow                  = 0;
    pFilter->FilterMaskIdHigh         =    0;
    pFilter->FilterMaskIdLow             =    0;
    pFilter->FilterFIFOAssignment = CAN_FILTER_FIFO0;
    pFilter->FilterBank                     = 0;
    pFilter->FilterMode                     = CAN_FILTERMODE_IDMASK;
    pFilter->FilterScale                     = CAN_FILTERSCALE_32BIT;
    pFilter->FilterActivation         = ENABLE;
    pFilter->SlaveStartFilterBank = 0;
}

void vApp_User_CAN_Configuration(void)
{
	/*----------------- CAN��ʼ������ --------------------------*/
	vApp_CAN_Configuration(&hCAN1_TxHeader, &hCAN1_Filter,
	/* TxHeader ������� */
	/* StdId ExtId IDE RTR DLC */
	0x141, 0, CAN_ID_STD, CAN_RTR_DATA, 8,
	/* Filter ������� */
	/* IdHigh IdLow MaskIdHigh MaskIdLow FIFOAssignment Bank Mode Scale Activation SlaveStartFilterBank */
	0, 0, 0, 0, CAN_FILTER_FIFO0, 0, CAN_FILTERMODE_IDMASK, CAN_FILTERSCALE_32BIT, ENABLE, 0);
}


void vApp_CAN_Configuration(CAN_TxHeaderTypeDef    * pTxHeader,
                                                        CAN_FilterTypeDef     * pFilter,
                                                        uint32_t                             StdId, 
                                                        uint32_t                             ExtId, 
                                                        uint32_t                             IDE, 
                                                        uint32_t                             RTR, 
                                                        uint32_t                             DLC,
                                                        uint32_t                             IdHigh,
                                                        uint32_t                             IdLow,
                                                        uint32_t                             MaskIdHigh,
                                                        uint32_t                             MaskIdLow,
                                                        uint32_t                             FIFOAssignment,
                                                        uint32_t                             Bank,
                                                        uint32_t                             Mode,
                                                        uint32_t                             Scale,
                                                        uint32_t                             Activation,
                                                        uint32_t                             SlaveStartFilterBank)
{
    /*-1- ��ʼ��TxHeader��� ----------------------------------------*/
    vApp_CAN_TxHeader_Init(pTxHeader, StdId, ExtId, IDE, RTR, DLC);
    
    /*-2- ��ʼ���˲������ ------------------------------------------*/
    vApp_CAN_Filter_Init(pFilter, IdHigh, IdLow, MaskIdHigh, MaskIdLow, FIFOAssignment, Bank, Mode, Scale, Activation, SlaveStartFilterBank);
    HAL_CAN_ConfigFilter(&hcan1, pFilter);
    
    /*-3- ����CAN ---------------------------------------------------*/
    while(HAL_CAN_Start(&hcan1) != HAL_OK )
    {
        //printf("\nCAN_Start Failed!!");
        HAL_Delay(100);
    }
    //printf("\nCAN_Start Success!!");
    
    /*-4- ʹ���ж�֪ͨ ----------------------------------------------*/
    HAL_CAN_ActivateNotification(&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);
}


void vApp_User_CAN1_TxMessage(uint8_t aTxData[], uint8_t DLC)
{
	vApp_CAN_TxMessage(&hcan1, &hCAN1_TxHeader, aTxData, DLC);
}

void vApp_CAN_TxMessage(CAN_HandleTypeDef *hcan, CAN_TxHeaderTypeDef * pTxHeader, uint8_t aData[], uint8_t DLC)
{
	uint32_t Tx_MailBox;
	/*-1- �������ݶγ��� ----------------------------------------*/
	pTxHeader->DLC    =    DLC;
	/*-2- ����aData ---------------------------------------------*/
	HAL_CAN_AddTxMessage(hcan, pTxHeader, aData, &Tx_MailBox);
//    while(HAL_CAN_AddTxMessage(hcan, pTxHeader, aData, &Tx_MailBox) != HAL_OK)
//    {
//        HAL_Delay(100);
//    }
}

/*******************************************************************************
* Function Name  : HAL_CAN_RxFifo0MsgPendingCallback
* Description    : ��Ϣ���ջص�����
* Input          : hcan
* Output         : None
* Return         : None
****************************************************************************** */
//void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan)
//{
//    uint8_t aRxData[8], i;
//    
//    if(HAL_CAN_GetRxMessage(hcan, CAN_RX_FIFO0, &hCAN1_RxHeader, aRxData) == HAL_OK)
//    {
//        //printf("\nGet Rx Message Success!!\nData:");
//        //for(i=0; i<8; i++)
//            //printf("%d", aRxData[i]);
//    }
//}
