void FACEID(void) {
	buzzer(1);
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","Pls Press DOWN");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed == '*')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			status++;
			status = (status > 1) ? (-1) : status;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Add FaceID");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Remove FaceID");
				break;
			default:
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Back");
				break;
			}
		}
		if (key_pressed == '#')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			switch (status)
			{
			case 0:
				uint8_t FaceID = InputID_FACE();
				if (checkfaceid(FaceID) != 0)
				{
					CLCD_I2C_Display(&LCD1,"    FACEID "," Face Existed ");
					buzzer(3);
					HAL_Delay(1000);
				}
				else
				{
					addface(FaceID);
				}
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Add FaceID");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
				uint8_t statusrm = 0;
				uint8_t backrm = 1;
				while (backrm == 1)
				{
					key_pressed = KeyPad_WaitForKeyGetChar(10);
					if (exitmenu == 0)
					{
						CLCD_I2C_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (key_pressed == '*')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						statusrm++;
						statusrm = (statusrm > 2) ? 0 : statusrm;
						switch (statusrm)
						{
						case 1:
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Remove 1 Face");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Remove ALL");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Back");
							break;
						}
					}
					if (key_pressed == '#')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						switch (statusrm)
						{
						case 1:
							uint8_t FaceID = InputID_FACE();
							if (checkfaceid(FaceID) == 0)
							{
								CLCD_I2C_Display(&LCD1, "    FaceID ", "  Do Not Exist");
								buzzer(3);
								HAL_Delay(1000);
							}
							else
							{
								removeface(FaceID);
								CLCD_I2C_Display(&LCD1,"REMOVE FACEID ","   SUCCESSFUL  ");
								HAL_Delay(1000);
							}
							CLCD_I2C_Display(&LCD1,"FACEID: REMOVE","=> Remove 1 Face");
							break;
						case 2:
							sprintf(Tx_Buffer , "Del.ALL" );
							CDC_Transmit_FS(Tx_Buffer, 7);
							CLCD_I2C_Display(&LCD1, "WAITING....", "");
							exitmenu = 60;
							memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
							while(exitmenu != 0){
								if(Rx_Buffer[0] == 'T'){
									CLCD_I2C_Display(&LCD1, "REMOVE ALL FACE","   SUCCESSFUL  ");
									HAL_Delay(2000);
									memset(Rx_Buffer, 0, sizeof(Rx_Buffer));
									break;
								}
							}
							exitmenu = 0;
							break;
						default:
							backrm=0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1,"FACEID SETTINGS ","=> Remove FaceID");
				break;
			default:
				exitmenu=0;
				break;
			}
		}
	}
	CLCD_I2C_Clear(&LCD1);
}
