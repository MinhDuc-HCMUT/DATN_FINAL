uint32_t CheckKey(uint8_t key)
{
    uint32_t pt = StartAddressFingerID;
    while (Flash_Read_Byte(pt) != 0xFF)
    {
        if (*(uint8_t *)(pt) == key)
            return pt;
        }
        pt = pt + 1;
    }
    return 0;
}
void adduid(uint8_t key)
{
	uint32_t pt = StartAddressFingerID;
	while (Flash_Read_Byte(pt) != 0xFF)
	{
		pt = pt + 1;
	}
    setaddress();
	FingerID[0] = key;
	Flash_Write_Array(pt, FingerID, 1);
}
