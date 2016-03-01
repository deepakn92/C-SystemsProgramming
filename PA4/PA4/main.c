//
//  main.c
//  PA4
//
//  Created by Deepak Nalla on 11/2/15.
//  Copyright (c) 2015 Deepak Nalla. All rights reserved.
//

#include <stdio.h>
void swap ( float *x, float *y);


void main (){
    float x = 3.5;
    float y = 4.5;
    printf( "Before: x %f , y %f\n", x,  y);
    swap( &x, &y);
    printf( "After: x %f , y %f", x, y);
  //  return 0;
}

void swap ( float *x, float *y)
{
    float temp=    = *y;
    *y = temp;
}




