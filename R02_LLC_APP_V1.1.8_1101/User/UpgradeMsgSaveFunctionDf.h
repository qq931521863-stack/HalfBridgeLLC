/******************************************************************************
    文件名称:   UpgradeMsgSaveFunctionDf.h
    版本号  :   v1.0
    作者    :   无锡芯创
    日期    :   20290608
    淘宝店铺:   https://shop252056293.taobao.com/
******************************************************************************/
#ifndef _UpgradeMsgSaveFunctionDf_H
#define _UpgradeMsgSaveFunctionDf_H
#include "stdint.h"
#include "HostFlashMemoryMap.h"
#include "HostBootState.h"

#define SWAP16(value) (uint16_t)(((value) << 8) | (((value) >> 8) & 0xFF))
#define SWAP32(value) (uint32_t)((((uint8_t*)&(value))[0] << 24) | (((uint8_t*)&(value))[1] << 16) | (((uint8_t*)&(value))[2] << 8) | ((uint8_t*)&(value))[3])

#define DF_EXT_FLASH_PACK_STORE_64K_BLOCKS              32U
#define DF_EXT_FLASH_PACK_STORE_FIRST_64K_BLOCK_IDX     0x30U

#define DF_CMD_START_ADDR_OFFSET                        2
#define DF_CMD_DATA_OFFSET                              6

/*
 * DevType 全工程统一定义（须与 从机/RS485(UART1)升级、从机/IAP Bootloader 一致）：
 *   0x0001 MASTET      — CAN1FD 主控(G474) 自身
 *   0x0002 SLAVE1      — RS485 从机；CAN1FD 合包后 RS485 转发必须用此值
 *   0x0003 MERGED_PACK — CAN 合成包（仅 CAN 下载阶段）
 */
#define Df_UPGRADE_DEV_MASTET                           0x0001
#define Df_UPGRADE_DEV_SLAVE1                           0x0002
#define Df_UPGRADE_DEV_MERGED_PACK                      0x0003



#define DF_UART_UPGRADE_ACK_INIT  				        0xFFFF        //串口升级应答初值
/************************升级模式应答定义***********************************/
#define DF_INTO_UPGRADE_MODE_SUCCEED_ACK                0x0000      //进入升级模式成功应答
#define DF_INTO_UPGRADE_MODE_FAIL_ACK                   0x0001      //进入升级模式失败应答
/************************启动升级应答***********************************/
#define DF_START_UPGRADE_SUCCEED_ACK                    0x0000      //启动升级成功应答
#define DF_START_UPGRADE_PACK_DATA_NUM_OVERSIZE_ACK     0x0001      //升级包数量超限应答
#define DF_START_UPGRADE_BIN_FILE_OVERSIZE_ACK          0x0002      //升级bin文件超限应答
#define DF_START_UPGRADE_OTHER_ERROR_ACK                0x0004      //启动升级其它错误

/************************文件传输分包应答***********************************/
#define DF_FILE_TRANSFER_FRAME_DATA_WRITE_FLASH_SUCCEED_ACK     0x0000  //分包数据写入FLASH成功
#define DF_FILE_TRANSFER_FRAME_DATA_WRITE_FLASH_ERROR_ACK       0x0001  //分包数据写入FLASH失败
#define DF_FILE_TRANSFER_FRAME_DATA_LEN_ERROR_ACK               0x0002  //分包数据长度错误
#define DF_FRAME_DATA_HAVE_BEEN_WRITE_FLASH_ACK                 0x0004  //分包数据已写入FLASH
#define DF_FRAME_DATA_SER_NUM_ERROR_ACK                         0x0008  //分包序号错误
#define DF_FILE_TRANSFER_FRAME_DATA_OTHER_ERROR_ACK             0x0010  //分包其它错误

/************************文件传输结束应答***********************************/
#define DF_FILE_TRANSFER_END_BIN_FILE_REC_SUCCEED_ACK           0x0000  //bin文件接收成功
#define DF_FILE_TRANSFER_END_FILE_TRANSFER_END_BIN_LEN_ERORR_ACK 0x0002  //结束包长度和bin长度不符
#define DF_FILE_TRANSFER_END_BIN_CRC_ERROR_ACK                  0x0001  //bin文件crc错误

#define DF_INTEGER_K_SIZE_VALUE                     1024                //K字节整数值
#define DF_INTEGER_2K_SIZE_VALUE                    2048                //2K字节整数值

#define DF_INQUIRE_UPGRADE_UPGRADING_STATUS_ACK     0x0001              //正在升级
#define DF_INQUIRE_UPGRADE_UPGRADE_SUCCEED_ACK      0x0000              //升级成功
#define DF_INQUIRE_UPGRADE_UPGRADE_FAIL_ACK         0x0002              //升级失败


#define DF_INTO_UPGRADEMODE_STEP                    0x01                //步骤1：进入升级模式
#define DF_START_UPGRADE_STEP                       0x02                //步骤2：开始升级
#define DF_UPGRADE_FILE_TRANSFER_STEP               0x03                //步骤3：文件传输
#define DF_UPGRADE_FILE_TRANSFER_END_STEP           0x04                //步骤4：传输结束
#define DF_UPGRADE_SCHEDULE_INQUIRE_STEP            0x05                //步骤5：进度查询

#define DF_WAIT_DOWNLOAD_UPGRADR_FILE_STATUS        0x5AB1              //等待下载升级文件
#define DF_WAIT_INTO_UPGRADR_STATUS                 0x5AB2              //等待进入升级
#define DF_UPGRADR_FINISH_STATUS                    0x5AB3              //升级完成
#define DF_UPGRADE_FAIL_STATUS                      0x5AB4              //升级失败
#define DF_NOT_UPGRADE_STATUS                       0x5AB5              //未升级

#define  DF_ERASE_FLASH_WAIT_INTO_ERASE_STATUS      0x00
#define  DF_ERASE_FLASH_ERASING_STATUS              0x01
#define  DF_ERASE_FLASH_ERASE_FINISH_STATUS         0x02

#define DF_INTO_UPGRADE_MODE_CMD                    0x55AA              //进入升级模式命令
#define DF_UPGRADE_START_CMD                        0x55AA              //升级开始命令

#define DF_UPGRADE_FILE_VAILE_STATUS                0xAA                //升级文件有效标志
#define DF_UPGRADE_FILE_INVAILE_STATUS              0x55                //升级文件无效标志

#define DF_CHS_BLOCK_A                              0x55
#define DF_CHS_BLOCK_B                              0xAA

#define DF_SYS_NOT_UPGRADE_STATUS  				    0xAA 		        //系统未升级  
#define DF_SYS_WAIT_DOWNLOAD_FIRMWARE_STATUS  		0x01		        // 系统等待下载固件
#define DF_SYS_DOWNLOAD_FIRMWARE_FINISH_STATUS  		0x02		        //下载固件完成
#define DF_SYS_DOWNLOAD_FIRMWARE_FILL_STATUS  		0x03		        //写入固件中
#define DF_SYS_VERIFY_NEW_FIRMWARE_FILL_STATUS  		0x04		        //校验新固件中
#define DF_SYS_WRITE_NEW_FIRMWARE_FINISH_STATUS  	0x05		        //新固件写入Flash完成
#define DF_SYS_WRITE_NEW_FIRMWARE_FILL_STATUS  		0x06		        //写入新固件到flash中


// 升级过程消息体
#pragma pack (1)//单字节对齐
typedef struct 
{
    uint8_t  u8UpgradeFlag;                         //升级标志
    
    uint8_t u8BlockABinFileValidFlag;               //A区bin文件有效标志
    uint8_t u8BlockBBinFileValidFlag;               //B区bin文件有效标志
    
    uint8_t  u8FlashUPtoBlock;                      //正在升级到哪个块
    uint8_t  u8FlashUserBlock;                      //当前APP运行块
    
    uint16_t u16UpgradeState;                       // 升级状态
    
    uint32_t u32BlockABinFileLength;                //A区bin文件长度  
    uint32_t u32BlockBBinFileLength;                //B区bin文件长度 
    
    uint16_t u16BlockABinFlieCrc16;                 //A区CRC16校验值
    uint16_t u16BlockBBinFlieCrc16;                 //B区CRC16校验值
    
    uint32_t u32BlockABinFlieCrc32;                 //A区CRC32校验值
    uint32_t u32BlockBBinFlieCrc32;                 //B区CRC32校验值
    

    uint16_t u16BlockAHardwareVersion;      		//A区硬件版本
    uint16_t u16BlockAFirmwareVersion;      		//A区固件版本
    
    uint16_t u16BlockBHardwareVersion;      		//B区硬件版本
    uint16_t u16BlockBFirmwareVersion;      		//B区固件版本
	
	/****************************************************************
				     0xAA:系统未升级  
					 0x01:等待下载固件
					 0x02:下载固件完成
					 0x03:写入固件中
					 0x04:校验新固件中
		             0x05:新固件写入Flash完成
					 0x06:写入新固件到flash中
	****************************************************************/
	uint8_t  u8IntoUpgradFlag;

    uint8_t u8ImageA1Valid;
    uint8_t u8ImageA2Valid;
    uint8_t u8ImageB1Valid;
    uint8_t u8ImageB2Valid;
    uint8_t u8UpgradePhase;
    uint8_t u8OtaServiceFlag;
    
    uint16_t u16ProgMsgCRC16Value;
}_StProgStatusMsg;
#pragma pack ()//恢复默认对齐


#pragma pack (1)//单字节对齐
typedef struct
{
  uint32_t  u32DataTab[11];
}_Stu32ProgStatus;
#pragma pack ()//恢复默认对齐


typedef union
{
    _StProgStatusMsg  StProgStatusMsg;
    _Stu32ProgStatus  Stu32ProgStatus;
    uint8_t u8Buf[44];
}_StProgStatus;


// 系统状态体定义
#pragma pack (1)//单字节对齐
typedef struct 
{
    uint8_t  u8SysStateFlag;                    //系统状态标志
    uint16_t u16SysUpgradeState;                // 升级状态
    uint8_t u8ReservedBuf[11];                  // 预留字节
    uint16_t u16CRC16Value;                     //结构体CRC16校验值
}_StSysStatus;
#pragma pack ()//恢复默认对齐

typedef struct
{
  uint32_t  u32DataTab[4];
}_StSysStatusBuf;

typedef union
{
    _StSysStatusBuf  StSysStatusBuf;
    _StSysStatus     StSysStatus;
    uint8_t u8Buf[16];
    uint16_t u16Buf[8];
}_utSysStatusMsg;

extern _utSysStatusMsg utSysStatusMsg;
extern _StProgStatus   StMasterProgStatus;      //主控区升级消息
extern _StProgStatus   StSlave1ProgStatus;      //从控1升级消息

extern void SaveWriteUpgradeFileMsg(_StProgStatus *Prog_Status,uint32_t u32Addr);
extern uint16_t ReadWriteUpgradeFileMsg(_StProgStatus *Prog_Status,uint32_t u32Addr);
extern void Sys_software_reset(void);
extern void DisenabledInterrupt(void);
extern void EnabledInterrupt(void);
#endif
