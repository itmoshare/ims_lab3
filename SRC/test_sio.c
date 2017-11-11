/*--------------------------------------------------------------

--------------------------------------------------------------*/

#include "aduc812.h"
#include "sio.h"
#include "led.h"
#define NEWLINE 0x0A
#define DEFAULT_VAL	0xFF
#define MAX_RESULT 0x63

//Global variables for mathematic construction
unsigned char outSymbol, result, number, inputCount, firstDigitMask, startOutput, outputCount;


/**----------------------------------------------------------------------------
                        moderes()
-------------------------------------------------------------------------------

----------------------------------------------------------------------------- */
void resetState() {		
		result = 0;
		inputCount = 1;
		startOutput = 0;
		outputCount = 0;
}

/**----------------------------------------------------------------------------
                        mistake()
-------------------------------------------------------------------------------

----------------------------------------------------------------------------- */
void mistake() {
			EA = 0;
			ES = 0;
			type ( "Mistake has happend\n" );
			leds( DEFAULT_VAL );
			resetState();
			EA = 1;
			ES = 1;
}


/**----------------------------------------------------------------------------
                            SIO_ISR
-------------------------------------------------------------------------------
-----------------------------------------------------------------------------*/
void SIO_ISR( void ) __interrupt ( 4 )
{
	unsigned char r_buf;

	if( TI ) //готов к передаче
	{
		if(outputCount < 8) 
		{
			
			outSymbol = (result & firstDigitMask) + 0x30;
			if(outSymbol > 0x30)
				startOutput = 1;
			if(startOutput)
				SBUF = outSymbol;
			outputCount++;
			result = result << 1;
			TI = 1; //продолжаем вывод
		}
		else 
		{//зкончили вывод
			SBUF = NEWLINE;
			resetState();
			TI=0;
		}
	}

	if( RI ) //принял байт
	{
		r_buf = SBUF;
		leds( r_buf ); 
		RI = 0;

		if(r_buf == NEWLINE)
		{
			TI=1;
		}
		//check if number
		else if(r_buf >= 0x30 && r_buf <= 0x39)
		{
			number = r_buf - 0x30;

			if (inputCount > 3)
			{
				mistake();//число больше 255
			}
			else if (inputCount == 3 && result > 25)
			{
				mistake(); //число больше 255
			} 
			else if (inputCount == 3 && result == 25 && number > 5)
			{
				mistake(); //число больше 255
			} 
			else
			{
				result = result * 10 + number; //нормальное число, прибавляем к результату	
				inputCount++;	
				RI=1;	
			}
		} 
		else 
		{
			mistake();//не число и не новая строка
		}		
	}
}

/**----------------------------------------------------------------------------
                            SetVectir
-------------------------------------------------------------------------------

-----------------------------------------------------------------------------*/
void SetVector(unsigned char __xdata * Address, void * Vector)
{
	unsigned char __xdata * TmpVector;
	*Address = 0x02;
	TmpVector = (unsigned char __xdata *) (Address + 1);
	*TmpVector = (unsigned char) ((unsigned short)Vector >> 8);
	++TmpVector;
	*TmpVector = (unsigned char) Vector;
}

void typeTriple(char charToType) 
{
	unsigned char i;
	for(i=0; i<3;i++) 
	{
		wsio(charToType);
	}
}

void triple(char charToTriple) 
{
	//41-5a big latins
	//61-7a small latins
	//80-9f big cyrillics
	//a0-af small cyrillic
	//e0-Ef small cyrillic
	//30-39 numbers(0-9)
	
	if(charToTriple >= 0x61 && charToTriple <= 0x7A)
	{
		typeTriple(charToTriple - 0x20);
	}
	else if(charToTriple >= 0x80 && charToTriple <= 0x9F)
	{
		if(charToTriple <= 0x8A)
			typeTriple(charToTriple + 0x20);
		else
			typeTriple(charToTriple + 0x30);
	}
	else 
	{
		typeTriple(charToTriple);
	}
}

void main( void )
{
	unsigned char dig, i, dipval;

	SetVector( 0x2023, (void *)SIO_ISR );
	init_sio( S9600 );

	resetState();
	
	firstDigitMask = 0x7F;

    while( 1 )
    {
		dipval = readdip(); // считываем значение переключателей
		if ( dipval == DEFAULT_VAL ) // первый режим - утроение
		{
			EA = 0;
			ES = 0;

			if( rsiostat() )
			{
				dig = rsio();
				triple(dig);
			}
		}
		else // второй режим - включаем прерывания
		{
			EA=1;
			ES=1;
		}
	}
}