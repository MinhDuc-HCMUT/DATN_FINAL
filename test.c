void RFID(void)
{
	buzzer(1);
	exitmenu = Delaymenu;
	uint8_t status = -1;
	CLCD_I2C_Display(&LCD1," RFID SETTINGS ","Pls Press DOWN");
	while (exitmenu )
	{
		char key_pressed = KeyPad_WaitForKeyGetChar(10);
		if (key_pressed =='*')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			status++;
			status = (status > 3) ? (-1) : status;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Add Card");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Remove 1 Card");
				break;
			case 2:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Remove All");
				break;
			case 3:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Check Card");
				break;
			default:
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Back");
				break;
			}
		}
		if (key_pressed =='#')
		{
			buzzer(1);
			exitmenu = Delaymenu;
			switch (status)
			{
			case 0:
				CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
				uint8_t statusadd = -1;
				uint8_t back = 1;
				while (back == 1)
				{
					key_pressed = KeyPad_WaitForKeyGetChar(10);
					if (exitmenu == 0)
					{
						CLCD_I2C_Clear(&LCD1);
						HAL_Delay(1000);
						return;
					}
					if (key_pressed =='*')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						statusadd++;
						statusadd = (statusadd > 1) ? (-1) : statusadd;
						switch (statusadd)
						{
						case 0:
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> Admin Card");
							break;
						case 1:
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> User Card");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> Back");
							break;
						}
					}
					if (key_pressed =='#')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						switch (statusadd)
						{
						case 0:
							uint8_t AdminID = InputID_ADMIN();
							uint8_t keyadd_admin = (1 << 7) + AdminID;
							if (CheckKey(keyadd_admin)!=0)
							{
								CLCD_I2C_Display(&LCD1," ID is existing"," Pick another ID");
								buzzer(3);
								HAL_Delay(1000);
							}
							else 
							{
								adduid(keyadd_admin);
							}
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> Admin Card");
							break;
						case 1:
							uint8_t UserID = InputID_USER();
							uint8_t keyadd_user = (0 << 7) + UserID;
							if (CheckKey(keyadd_user)!=0)
							{
								CLCD_I2C_Display(&LCD1," ID is existing"," Pick another ID");
								buzzer(3);
								HAL_Delay(1000);
							}
							else 
							{
								adduid(keyadd_user);
							}
							CLCD_I2C_Display(&LCD1,"CARD: ADD","=> User Card");
							break;
						default:
							back = 0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Add Card");
				break;
			case 1:
				CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
				uint8_t statusremove = -1;
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
					if (key_pressed =='*')
					{
						buzzer(1);
						exitmenu = Delaymenu;
						statusremove++;
						statusremove = (statusremove > 1) ? (-1) : statusremove;
						switch (statusremove)
						{
						case 0:
							CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Select Card");
							break;
						case 1:
							CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Scan Card");
							break;
						default:
							CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Back");
							break;
						}
					}
					if (key_pressed =='#')
					{
						buzzer(1);
						CLCD_I2C_Display(&LCD1,"  Please Press","      DOWN");
						exitmenu = Delaymenu;
						switch (statusremove)
						{
						case 0:
							uint8_t statusrm1 = -1;
							uint8_t backrm1 = 1;
							while (backrm1 == 1)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (exitmenu == 0)
								{
									CLCD_I2C_Clear(&LCD1);
									HAL_Delay(1000);
									return;
								}
								if (key_pressed =='*')
								{
									buzzer(1);
									statusrm1++;
									statusrm1 = (statusrm1 > 1) ? (-1) : statusrm1;
									switch (statusrm1)
									{
									case 0:
										CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM Admin Card");
										break;
									case 1:
										CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM User Card");
										break;
									default:
										CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> Back");
										break;
									}
								}
								if (key_pressed =='#')
								{
									buzzer(1);
									exitmenu = Delaymenu;
									switch (statusrm1)
									{
									case 0: 
										uint8_t AdminID = InputID_ADMIN();
										uint8_t keyadd_admin = (1 << 7) + AdminID;
										if (CheckKey(keyadd_admin)==0)
										{
											CLCD_I2C_Display(&LCD1,"ID doesnt exist"," Pick another ID");
											buzzer(3);
											HAL_Delay(1000);
											CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM Admin Card");
										}
										else 
										{
											removeuid(CheckKey(keyadd_admin));
											CLCD_I2C_Display(&LCD1,"REMOVE ADMIN CARD","   SUCCESSFUL  ");
											HAL_Delay(1000);
											if (checkcountUID() == 0)
											{
												startadd();
												exitmenu = 0;
											}
										}
										CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM Admin Card");
										break;
									case 1:
										uint8_t UserID = InputID_USER();
										uint8_t keyadd_user = (0 << 7) + UserID;
										if (CheckKey(keyadd_user)==0)
										{
											CLCD_I2C_Display(&LCD1,"ID doesnt exist"," Pick another ID");
											buzzer(3);
											HAL_Delay(1000);
										}
										else 
										{
											removeuid(CheckKey(keyadd_user));
											CLCD_I2C_Display(&LCD1,"REMOVE USER CARD","   SUCCESSFUL  ");
											HAL_Delay(1000);
											if (checkcountUID() == 0)
											{
												startadd();
												exitmenu = 0;
											}
										}
										CLCD_I2C_Display(&LCD1,"MODE: RM SELECT","=> RM User Card");
										break;
									default:
										backrm1 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Remove 1 Card");
							break;
						case 1:
							CLCD_I2C_Display(&LCD1,"PLS SCAN CARD","=> Back");
							uint8_t rmquet = 1;
							while (rmquet)
							{
								key_pressed = KeyPad_WaitForKeyGetChar(10);
								if (TM_MFRC522_Check(CardID) == MI_OK)
								{
									if (CheckListUID(CardID) != 0)
									{
										removeuid(CheckKey(CheckListUID(CardID)));
										CLCD_I2C_Display(&LCD1,"  DELETE CARD ","   SUCCESSFUL  ");
										HAL_Delay(1000);
										if (checkcountUID() == 0)
										{
											startadd();
											rmquet = 1;
											exitmenu = 0;
											return;
										}else{
											CLCD_I2C_Display(&LCD1,"PLS SCAN CARD","=> Back");
										}
									}
									else
									{
										CLCD_I2C_Display(&LCD1, "   This card","  Do not exist");
										buzzer(3);
										HAL_Delay(1000);
										CLCD_I2C_Display(&LCD1,"PLS SCAN CARD","=> Back");
									}
								}
								if (key_pressed =='#')
								{
									buzzer(1);
									rmquet = 0;
								}
							}
							CLCD_I2C_Display(&LCD1,"MODE: REMOVE 1","=> Scan Card");
							break;
									default:
										backrm1 = 0;
										break;
									}
								}
							}
							CLCD_I2C_Display(&LCD1,"CARD: REMOVE","=> Remove 1 Card");
							break;
						// case 1:
						// 	remoall();
						// 	startadd();
						// 	exitmenu = 0;
						// 	break;
						default:
							backrm = 0;
							break;
						}
					}
				}
				CLCD_I2C_Display(&LCD1,"CARD: REMOVE","=> Remove 1 Card");
				break;
			case 2:
				remoall();
				startadd();
				exitmenu = 0;
				break;
			case 3:
				checkthe();
				CLCD_I2C_Display(&LCD1," RFID SETTINGS ","=> Check Card");
				break;
			default:
				exitmenu = 0;
				break;
	CLCD_I2C_Clear(&LCD1);
