void FINGER(void) {
	buzzer(1);
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1,"FINGER SETTING ","Pls Press DOWN");
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
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Add Finger");
				break;
			case 1:
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove Finger");
				break;
			default:
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Back");
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
			    add_finger();
				CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Add Finger");
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
							CLCD_I2C_Display(&LCD1,"FINGER: REMOVE","=> Remove 1 Finger");
							break;
						case 2:
							CLCD_I2C_Display(&LCD1,"FINGER: REMOVE","=> Remove All");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"FINGER: REMOVE","=> Back");
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
							remove_id_finger();
							CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove 1 Finger");
							break;
						case 2:
							remove_all_finger();
							CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove All");
                			break;
						default:
							backrm=0;
							break;
						}
					}
				}
	            CLCD_I2C_Display(&LCD1,"FINGER SETTING ","=> Remove Finger");
				break;
			default:
				exitmenu=0;
				break;
			}
		}
	}
	CLCD_I2C_Clear(&LCD1);
}
