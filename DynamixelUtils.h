//
// Created by trotfunky on 11/09/18.
//

#ifndef DYNAMIXEL_UTILS_H
#define DYNAMIXEL_UTILS_H

#include "Arduino.h"

/*
 * Protocol utilities
 */

//!Dynamixel Potocol v2 magic values
/*!
 * The enum contains useful values of the Dynamixel communication protocol 2.0. For example:
 * \li Minimum lengths of messages
 * \li Instruction values
 * \li Positions of useful data
 */
enum dynamixelV2{
    minPacketLenght = 12,       //!< With checksum
    minInstructionLength = 5,   //!< Without checksum
    minResponseLength = 5,      //!< Without checksum
    writeInstruction = 0x03,
    readInstruction,
    statusInstruction = 0x55,
    alertBit = 128,
    lengthLSBPos = 5,
    lengthMSBPos = 6,
    instructionPos = 7,
    responseParameterStart = 8
};

constexpr unsigned char v1Header[2] = {0xFF,0xFF};
constexpr unsigned char v2Header[4] = {0xFF,0xFF,0xFD,0x00};



/*
 * Data structures definitions
 */



//! Contains address and length of data.
/*!
 * \li Address is two bytes long, represented in little endian.
 * \li Length is the length in bytes of subsequent data
 *
 */
struct DynamixelAccessData {
    DynamixelAccessData(const uint8_t addressLSB,const uint8_t addressMSB,const uint8_t size) : address{addressLSB,addressMSB}, length(size)
    {}

    const uint8_t address[2];    //!< Two bytes, little endian memory address
    const uint8_t length;        //!< Length of data to read/write
};



//! Contains necessary data for data access and conversion
/*!
 * <br>It is a protected member of DynamixelMotor thus it can be re-used in child classes,
 * each child defining its own DynamixelAccessData and conversion factors.
 * <br>DynamixelMotor does not define any default value.
 * <br>The structure contains the following data:
 * \li Angle and velocity readings and targets access
 * \li Torque activation and reading access
 * \li ID and LED access
 * \li Angle, velocity and torque conversion factors
 * \li Motor ID
 *
 */
struct DynamixelMotorData {

    DynamixelMotorData(uint8_t newID,const DynamixelAccessData& idAccess,const DynamixelAccessData& ledAccess,
                       const DynamixelAccessData& torqueEnAccess, const DynamixelAccessData& currentTorqueAccess,
                       const DynamixelAccessData& goalAngleAccess,const DynamixelAccessData& currentAngleAccess,
                       const DynamixelAccessData& goalVelocityAccess,const DynamixelAccessData& currentVelocityAccess,
                       const float torqueConvertFactor,const float angleConvertFactor,const float velocityConvertFactor)
                        : motorID(newID), id(idAccess), led(ledAccess),
                          torqueEnable(torqueEnAccess), currentTorque(currentTorqueAccess),
                          goalAngle(goalAngleAccess), currentAngle(currentAngleAccess),
                          goalVelocity(goalVelocityAccess), currentVelocity(currentVelocityAccess),
                          valueToTorque(torqueConvertFactor), valueToAngle(angleConvertFactor),
                          valueToVelocity(velocityConvertFactor)
    {}

    uint8_t motorID; //!< Only object-specific attribute, used for ID changes

    /*!
     * \name Common attributes
     * Class-specific attributes : each kind of motor has its own. Ideally, every object of the same class should reference
     * the same DynamixelAccessData objects to minimize memory impact.
     */
    //!@{
    const DynamixelAccessData& id;
    const DynamixelAccessData& led;
    const DynamixelAccessData& torqueEnable;
    const DynamixelAccessData& currentTorque;
    const DynamixelAccessData& goalAngle;
    const DynamixelAccessData& currentAngle;
    const DynamixelAccessData& goalVelocity;
    const DynamixelAccessData& currentVelocity;

    const float valueToTorque;
    const float valueToAngle;
    const float valueToVelocity;
    //!@}
};



//!Abstraction struct allowing protocol-independent sending and receiving
/*!
 * The main goal of this struct is to allow the DynamixelManager to send and receive messages without any knowledge of
 * the underlying protocols. Thus, we can use any motor with any protocol.
 */
struct DynamixelPacket {

    //!Packet without expected response : status packets will be ignored.
    DynamixelPacket(const unsigned char* packet, uint8_t length) : packet(packet), packetSize(length), responseSize(0)
    {}
    DynamixelPacket(const unsigned char* packet, uint8_t length, uint8_t responseLength) : packet(packet), packetSize(length), responseSize(responseLength)
    {}
    ~DynamixelPacket()
    {
        delete[] packet;
    }

    const unsigned char* packet;
    const uint8_t packetSize;           //!< Length of data to send through serial.
    const uint8_t responseSize;         //!< Expected response size. If too big, serial will timeout.
};



/*
 * Error detection functions
 */


/*!
 * This table is used during the checksum calcultion of the Dynamixel communication protocol 2
 * <br>See crc_compute(const unsigned char*, unsigned short) for details
 */
constexpr unsigned short crc_table[256] = {
        0x0000, 0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
        0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027, 0x0022,
        0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D, 0x8077, 0x0072,
        0x0050, 0x8055, 0x805F, 0x005A, 0x804B, 0x004E, 0x0044, 0x8041,
        0x80C3, 0x00C6, 0x00CC, 0x80C9, 0x00D8, 0x80DD, 0x80D7, 0x00D2,
        0x00F0, 0x80F5, 0x80FF, 0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1,
        0x00A0, 0x80A5, 0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1,
        0x8093, 0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
        0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197, 0x0192,
        0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE, 0x01A4, 0x81A1,
        0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB, 0x01FE, 0x01F4, 0x81F1,
        0x81D3, 0x01D6, 0x01DC, 0x81D9, 0x01C8, 0x81CD, 0x81C7, 0x01C2,
        0x0140, 0x8145, 0x814F, 0x014A, 0x815B, 0x015E, 0x0154, 0x8151,
        0x8173, 0x0176, 0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162,
        0x8123, 0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
        0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104, 0x8101,
        0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D, 0x8317, 0x0312,
        0x0330, 0x8335, 0x833F, 0x033A, 0x832B, 0x032E, 0x0324, 0x8321,
        0x0360, 0x8365, 0x836F, 0x036A, 0x837B, 0x037E, 0x0374, 0x8371,
        0x8353, 0x0356, 0x035C, 0x8359, 0x0348, 0x834D, 0x8347, 0x0342,
        0x03C0, 0x83C5, 0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1,
        0x83F3, 0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
        0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7, 0x03B2,
        0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E, 0x0384, 0x8381,
        0x0280, 0x8285, 0x828F, 0x028A, 0x829B, 0x029E, 0x0294, 0x8291,
        0x82B3, 0x02B6, 0x02BC, 0x82B9, 0x02A8, 0x82AD, 0x82A7, 0x02A2,
        0x82E3, 0x02E6, 0x02EC, 0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2,
        0x02D0, 0x82D5, 0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1,
        0x8243, 0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
        0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264, 0x8261,
        0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E, 0x0234, 0x8231,
        0x8213, 0x0216, 0x021C, 0x8219, 0x0208, 0x820D, 0x8207, 0x0202
};

//! Dynamixel Protocl v1 checksum
/*!
 * Classic ones complement of the packet sum.
 * @param packet_to_check
 * @param packet_size
 * @return 1 byte checksum
 */
static unsigned char v1Checksum(const unsigned char *packet_to_check, unsigned short packet_size)
{
    int tempSum = 0;
    for(int i=0;i<packet_size;i++)
    {
        tempSum += packet_to_check[i];
    }

    return(~tempSum & 0xFF);
}

//! Dynamixel Protocol v2 crc
/*!
 * Cyclic redundancy check from Dynamixel communication protocol 2. It is an error-detecting code.
 * <br>See http://emanual.robotis.com/docs/en/dxl/crc/ for details.
 * @param packet_to_check
 * @param packet_size
 * @return 2 Byte, little endian crc
 */
static unsigned short crc_compute(const unsigned char *packet_to_check, unsigned short packet_size)
{
    unsigned short crc_accum = 0;
    unsigned short i, j;

    for(j = 0; j < packet_size; j++)
    {
        i = ((crc_accum >> 8) ^ packet_to_check[j]) & 0xFF;
        crc_accum = (crc_accum << 8) ^ crc_table[i];
    }

    return crc_accum;
}

#endif //DYNAMIXEL_UTILS_H
