// resize bitmap smoother

#pragma once

class  CBitmapResize
{
public:
	CBitmapResize(void);
	virtual ~CBitmapResize(void);

	HBITMAP ScaleBitmapInt(HBITMAP hBmp, 
						   WORD wNewWidth, 
						   WORD wNewHeight);


private:

};
