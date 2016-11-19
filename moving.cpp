#include <dos.h>
#include <conio.h>
#include <fstream.h>
#include <iostream.h>

// mcga
// This functions turns on "graphics mode MCGA"  320 x 200 pixels
// Parameters: none
// Returns: none
void mcga()
{
  REGS sIn,sOut;
  sIn.h.ah=0;
  sIn.h.al=19;
  int86(0x10,&sIn,&sOut);
}

// background
// This functions "paints" a background with color "X"
// Parameters: none
// Returns: none
void background()
{
   char far *ptr = (char far *)0xA0000000;
   for (unsigned int iPixels=0;iPixels<64000;iPixels++,ptr++)
      *ptr = 9;
}

/*
paintPixel()
This function paints a specific pixel in the screen
Parameters: iRow, iColumn, iColor
Returns: None
*/
void paintPixel(int iRow, int iColumn, unsigned short int iColor)
{
    char far *ptr = (char far *)0xA0000000;
    ptr += 320*(iRow) + (iColumn);
    *ptr = iColor;
}

/*
readPixel
This function will read the data of a specific pixel in the screen
Parameters: iRow, iColumn, iColor
Returns: None
*/
void readPixel(int iRow, int iColumn, int &iColor)
{
    char far *ptr = (char far *)0xA0000000;
    ptr += 320*(iRow) + (iColumn);
    iColor = *ptr;
}

// paintHorLine
// This function paints a pixel in a coordinate (row,column), and those 100 pixels to its right with a provided color
// Parameters: iRow, iColumn, iColor, coordinate (row,column) and a pixel color
// Returns: none
void paintHorLine(int iRow, int iColumn, unsigned short int iColor)
{
   for(int iCounter = 0; iCounter <= 100; iCounter++)
   {
       paintPixel(iRow, iColumn + iCounter, iColor);
   }
}

// paintVertLine
// This function paints a pixel in a coordinate (row,column), and those 100 below it with a provided color
// Parameters: iRow, iColumn, iColor, coordinate (row,column) and a pixel color
// Returns: none
void paintVertLine(int iRow, int iColumn, unsigned short int iColor)
{
   for(int iCounter = 0; iCounter <= 100; iCounter++)
   {
       paintPixel(iRow + iCounter, iColumn, iColor);
   }
}

/*
paintRectangle
This function paints a rectangle of specific dimensions in a specific place in the screen
Parameters: iRow, iColumn, iWidth, iHeight, iColor
Returns: None
*/
void paintRectangle(int iRow, int iColumn, int iWidth, int iHeight, unsigned short int iColor)
{
  int iCounter;

  //Two loops will manage different lines in specific locations to produce a rectangle
  for(iCounter = 1; iCounter <= iHeight; iCounter++)
  {
    int iCounter2;
    for(iCounter2 = 1; iCounter2 <= iWidth; iCounter2++)
    {
      paintPixel(iRow+iCounter, iColumn+iCounter2, iColor);
    }
  }
}

//displaySprite
//This function will display a specific sprite with specific dimensions in a specific location
//Parameters: iArrSprite, iWidth, iHeight, iRow, iColumn
//Returns: None
void displaySprite(int iArrSprite[20][20], int iWidth, int iHeight, int iRow, int iColumn)
{
	int iRowCounter, iColumnCounter, iPixel;

	//The two loops will read every data in the Matrix
	for (iRowCounter = 0; iRowCounter < iHeight; iRowCounter++)
	{
		for (iColumnCounter = 0; iColumnCounter < iWidth; iColumnCounter++)
		{
		    iPixel = iArrSprite[iRowCounter][iColumnCounter];
		    if(iPixel != 0)
                paintPixel(iRow + iRowCounter, iColumn + iColumnCounter, iPixel);
		}
	}
}

/*
displaySpriteHFlip()
This function will display a specific sprite, but flipped horizontally
Parameters: iArrSprite, iWidth, iHeight, iRow, iColumn
Returns: None
*/
void displaySpriteHFlip(int iArrSprite[20][20], int iWidth, int iHeight, int iRow, int iColumn)
{
	int iRowCounter, iColumnCounter, iPixel;

	for (iRowCounter = 0; iRowCounter < iHeight; iRowCounter++)
	{
		for (iColumnCounter = 0; iColumnCounter < iWidth; iColumnCounter++)
		{
		    iPixel = iArrSprite[iRowCounter][(iWidth - 1) - iColumnCounter];
		    if(iPixel != 0)
                paintPixel(iRow + iRowCounter, iColumn + iColumnCounter, iPixel);
		}
	}
}

/*
readSprite()
This function will read a whole sprite from a DEF file
Parameters: cArrSprite, iMatSprite, iWidth, iHeight
Returns: None
*/
void readSprite(char cArrSprite[], int iMatSprite[20][20], int &iWidth, int &iHeight)
{
    //The file is opened and reads every line in the DEF file
	int iRowCounter, iColumnCounter, iPixel;
	ifstream fMyFile;
	fMyFile.open(cArrSprite);

	fMyFile >> iWidth;
	iWidth++;

	fMyFile >> iHeight;
	iHeight++;

	for (iRowCounter = 0; iRowCounter < iHeight; iRowCounter++)
	{
		for (iColumnCounter = 0; iColumnCounter < iWidth; iColumnCounter++)
		{
			fMyFile >> iPixel;
			iMatSprite[iRowCounter][iColumnCounter] = iPixel;
		}
	}

	fMyFile.close();
}

/*
saveBackup()
This function will save the background image of a certain dimensions in a specific location in the screen
Parameters: iMatSprite, iWidth, iHeight, iRow, iColumn
Returns: None
*/
void saveBackup(int iMatSprite[20][20], int iWidth, int iHeight, int iRow, int iColumn)
{
	int iRowCounter, iColumnCounter, iPixel;

	for (iRowCounter = 0; iRowCounter < iHeight; iRowCounter++)
	{
		for (iColumnCounter = 0; iColumnCounter < iWidth; iColumnCounter++)
		{
			readPixel(iRow+iRowCounter,iColumn+iColumnCounter,iPixel);
			iMatSprite[iRowCounter][iColumnCounter] = iPixel;
		}
	}
}

/*
checkKey
This function checks the key that is pressed in the keyboard
Parameters: cKey, cScanCode
Returns: !(sal.x.flags&64)
*/
int checkKey(char &cKey, char &cScanCode)
{
	struct REGS en,sal;
	en.h.ah = 1;
 	int86(22,&en,&sal);
 	if (!(sal.x.flags&64))
  	{
		en.h.ah = 0;
		int86(22,&en,&sal);
		cKey = sal.h.al;
		cScanCode= sal.h.ah;
	}
	return !(sal.x.flags&64);
}

/*
readUnderPixels()
This function will create an array that contains all the colors of all the pixels under the sprite
Parameters:
*/
void readUnderPixels(int iArrColision[20], int iRow, int iColumn, int iHeight, int iWidth)
{
    int iColor;

    for(int iCounter2 = 0; iCounter2 < iWidth; iCounter2++)
            {
                readPixel(iRow + iHeight, iColumn + iCounter2, iColor);
                iArrColision[iCounter2] = iColor;
            }
}

/*
checkStep()
This function will check if the sprite is stepping on a surface that can't be passed
Parameters: iArrColision, iWidth, iSteps
Returns: None
*/
void checkStep(int iArrColision[20], int iWidth, int &iSteps)
{
    for(int iCounter = 0; iCounter < iWidth; iCounter++)
    {
        if(iSteps != 1)
        {
            if(iArrColision[iCounter] == 8 || iArrColision[iCounter] == 2)
            {
                iSteps = 1;
            }
            else
            {
                iSteps = 0;
            }
        }
    }
}

/*
jump()
This function will execute a loop to animate movement in a jump, moving a maximum of 20 pixels up and 10 pixels to the left or right, depending of its direction
Parameters: iMatSprite, iWidth, iHeight, iRow, iColumn, iDirection
Returns: None
*/
void jump(int iMatSprite[20][20], int iWidth, int iHeight, int &iRow, int &iColumn, int iDirection)
{
    int iMatBackup[20][20], iSteps = 0, iColor, iArrColision[20];
	char cAscii, cScanCode;

	for(int iCounter = 0; iCounter <= 21; iCounter++)
        {
            saveBackup(iMatBackup, iWidth, iHeight, iRow, iColumn);

            if(iDirection == 77)
            {
                displaySpriteHFlip(iMatSprite, iWidth, iHeight, iRow, iColumn);
            }

            if(iDirection == 75)
            {
                displaySprite(iMatSprite, iWidth, iHeight, iRow, iColumn);
            }

            delay(20);
            displaySprite(iMatBackup, iWidth, iHeight, iRow, iColumn);

            readUnderPixels(iArrColision, iRow, iColumn, iHeight, iWidth);

            checkStep(iArrColision, iWidth, iSteps);

            if(iSteps == 0 || iCounter == 0)
            {
                if(iCounter <= 10 && iDirection == 77)
                {
                    iRow -= 2;
                    iColumn++;
                }

                if(iCounter <= 10 && iDirection == 75)
                {
                    iRow -= 2;
                    iColumn--;
                }

                if(iCounter > 10 && iDirection == 77)
                {
                    iRow += 2;
                    iColumn++;
                }

                if(iCounter > 10 && iDirection == 75)
                {
                    iRow += 2;
                    iColumn--;
                }
            }

            iSteps = 0;

        }
}

/*
checkColision()
This function will check if all the pixels next to the sprite match color 8 and return 1 if true, 0 if false
Parameters: iMatSprite, iWidth, iHeight, iRow, iColumn, iDirection
Returns: iCrashes
*/
void checkColision(int iArrColision[20], int iHeight, int &iCrashes)
{
    for(int iCounter = 0; iCounter < iHeight; iCounter++)
    {
        if(iCrashes != 1)
        {
            if(iArrColision[iCounter] == 8)
            {
                iCrashes = 1;
            }
            else
            {
                iCrashes = 0;
            }
        }
    }
}

/*
movement()
This function will move the sprite in one cycle
Parameters: iMatSprite, iMatBackup, iWidth, iHeight, &iRow, &iColumn
Returns: None
*/
void movement(int iMatSprite[20][20], int iMatBackup[20][20], int iWidth, int iHeight, int &iRow, int &iColumn, int iDirection)
{
    saveBackup(iMatBackup, iWidth, iHeight, iRow, iColumn);

		if(iDirection == 77)
        {
            displaySpriteHFlip(iMatSprite, iWidth, iHeight, iRow, iColumn);
        }

        if(iDirection == 75)
        {
            displaySprite(iMatSprite, iWidth, iHeight, iRow, iColumn);
        }

		delay(50);
		displaySprite(iMatBackup, iWidth, iHeight, iRow, iColumn);

}

/*
shoot()
This function will make the sprite shoot the power to its respective direction
Parameters: iMatPoder, iMatBackupPoder, iRow, iColumn, iDirection
Returns:None
*/
void shoot(int iMatPoder[20][20], int iRow, int iColumn, int iDirection, int iDistance)
{
    if(iDirection == 77)
    {
        for(int iCounter3 = 0; iCounter3 < iDistance; iCounter3++)
                         {
                            int iColumnPoder = iColumn + 14 , iMatBackupPoder[20][20];
                            saveBackup(iMatBackupPoder, 14, 8, iRow, iColumnPoder + iCounter3);
                            displaySprite(iMatPoder, 14, 8, iRow, iColumnPoder + iCounter3);
                            delay(5);
                            displaySprite(iMatBackupPoder, 14, 8, iRow, iColumnPoder + iCounter3);
                         }
    }

    if(iDirection == 75)
    {
        for(int iCounter3 = 0; iCounter3 < iDistance; iCounter3++)
                         {
                            int iColumnPoder = iColumn - 14, iMatBackupPoder[20][20];
                            saveBackup(iMatBackupPoder, 14, 8, iRow, iColumnPoder - iCounter3);
                            displaySpriteHFlip(iMatPoder, 14, 8, iRow, iColumnPoder - iCounter3);
                            delay(5);
                            displaySprite(iMatBackupPoder, 14, 8, iRow, iColumnPoder - iCounter3);
                         }
    }

}

/*
animate()
This function will execute a loop to animate and move the character depending on keys pressed
Parameters: iMatSprite, iWidth, iHeight
Returns: None
*/
void animate(int iMatSprite[20][20], int iMatPoder[20][20], int iWidth, int iHeight)
{
	int iMatBackup[20][20], iRow = 115, iColumn = 0, iDirection = 77, iColor, iArrColision[20], iCrashes = 0, iSteps, iDistance;
	char cAscii, cScanCode;
	unsigned short int iQuit = 0;

	while(iQuit == 0)
    {
		saveBackup(iMatBackup, iWidth, iHeight, iRow, iColumn);

		if(iDirection == 77)
        {
            displaySpriteHFlip(iMatSprite, iWidth, iHeight, iRow, iColumn);
        }

        if(iDirection == 75)
        {
            displaySprite(iMatSprite, iWidth, iHeight, iRow, iColumn);
        }

		delay(50);
		displaySprite(iMatBackup, iWidth, iHeight, iRow, iColumn);

		readUnderPixels(iArrColision, iRow, iColumn, iHeight, iWidth);

		//cout << iArrColision[2];

        checkStep(iArrColision, iWidth, iSteps);

        //cout << iSteps;

        if(iSteps == 0)
        {
            while(iSteps == 0)
            {
                iRow++;

                movement(iMatSprite, iMatBackup, iWidth, iHeight, iRow, iColumn, iDirection);

                readUnderPixels(iArrColision, iRow, iColumn, iHeight, iWidth);

                checkStep(iArrColision, iWidth, iSteps);
            }
        }
        iSteps = 0;


		//These commands will only be executed if a key is pressed
		//Each switch contains instructions to be made for every key pressed
		if (checkKey(cAscii, cScanCode))
        {
            switch(cScanCode)
            {
                case 75:  if(iColumn - 10 >= 0)
                        {
                              //Establishing the sprite's direction (75 == left)
                              iDirection = 75;

                              //This loop will create an array containing all the colors in the pixels directly to the left of the sprite
                              for(int iCounter = 0; iCounter < iHeight; iCounter++)
                                {   //The pixels read are directly to the left of the sprite
                                    readPixel(iRow + iCounter, iColumn - 1, iColor);
                                    iArrColision[iCounter] = iColor;
                                }

                                //The function will update the variable iCrashes which will be 1 if it crashes, 0 if it doesnt
                                checkColision(iArrColision, iHeight, iCrashes);

                                //If the iCrashes variable is 0, then the sprite will move
                                if(iCrashes == 0)
                                    iColumn -= 11;

                                //The variable must be returned to its default value.
                                iCrashes = 0;
                        }
                break;
                case 77:    //readPixel(iRow, iColumn + iWidth, iColor);
                            if(iColumn + iWidth + 10 <= 320)
                            {
                                //The direction of the sprite is established (77 == right)
                                iDirection = 77;

                                //This loop will create an array containing all the colors that are directly to the right of the sprite
                                for(int iCounter = 0; iCounter < iHeight; iCounter++)
                                {
                                    //The pixels that will be read are those directly to the right of the sprite
                                    readPixel(iRow + iCounter, iColumn + iWidth, iColor);
                                    iArrColision[iCounter] = iColor;
                                }

                                //The checkColision function will update the iCrashes variable. 0 if false and 1 if true
                                checkColision(iArrColision, iHeight, iCrashes);
                                //The sprite will move if the iCrashes variable is 0
                                if(iCrashes == 0)
                                    iColumn += 11;
                                //iCrashes must be returned to its default value
                                iCrashes = 0;
                            }
                break;
                case 72:  if(iDirection == 77)
                         {
                             if((iColumn + iWidth) + 20 <= 319)
                                {
                                    jump(iMatSprite, iWidth, iHeight, iRow, iColumn, iDirection);
                                }
                         }

                         if(iDirection == 75)
                         {
                             if(iColumn - 20 >= 0)
                                {
                                    jump(iMatSprite, iWidth, iHeight, iRow, iColumn, iDirection);
                                }
                         }
                break;
                case 80:  if(iRow + iHeight + 10 <= 200)
                          {
                            //iDirection = 80;
                            iRow += 10;
                          }
                break;
                case 57: saveBackup(iMatBackup, iWidth, iHeight, iRow, iColumn);
                         if(iDirection == 77)
                            {
                                displaySpriteHFlip(iMatSprite, iWidth, iHeight, iRow, iColumn);
                                iDistance = 320 - (iColumn + 28);
                            }

                         if(iDirection == 75)
                            {
                                displaySprite(iMatSprite, iWidth, iHeight, iRow, iColumn);
                                iDistance = (iColumn - 28);
                            }

                         shoot(iMatPoder, iRow, iColumn, iDirection, iDistance);

                         displaySprite(iMatBackup, iWidth, iHeight, iRow, iColumn);

                         break;
                case 16: iQuit = 1;
                break;
            }
        }
	}
}

/*
main()

This function will execute all necessary instructions for the program to run correctly

Parameters: None
Returns: Zero
*/
int main()
{
    int iMatSprite[20][20], iMatBackup[20][20], iMatPoder[20][20], iWidth, iHeight;

    //Calling mcga function
    mcga();

    //Calling background
    background();

    //calling paintrectangle function
    paintRectangle(160, 0, 320, 40, 2);

    //calling paintrectangle to paint a platform
    paintRectangle(150, 100, 40, 5, 8);

    //calling paintrectangle to paint a platform
    paintRectangle(140, 150, 40, 5, 8);

    //calling readsprite function
    readSprite("poder.def", iMatPoder, iWidth, iHeight);

    //calling readsprite function
    readSprite("copycatr.DEF", iMatSprite, iWidth, iHeight);

    //calling animate function
    animate(iMatSprite, iMatPoder, iWidth, iHeight);


    return 0;
}
