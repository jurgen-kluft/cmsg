
/*
 * PlusCallback 1.7
 * Copyright (c) 2009-2010 Lewis Van Winkle
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software
 * in a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 * misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

#include "cmsg/c_delegate.h"

struct Dog
{
    void Bark(int volume){};
};

struct Cat
{
    void Meow(int volume){};
};

Dog spot, rover; //We have two dogs
Cat felix; //and one cat.

//Define a normal function.
void Func(int a){};


static void DelegateTests()
{
   //Define a callback to a function returning void and taking
    //one int parameter.
    ncore::Callback1<void, int> speak;

    //Point this callback at spot's Bark method.
    speak.Reset(&spot, &Dog::Bark);
    speak(50); //Spot barks loudly.

    speak.Reset(&rover, &Dog::Bark);
    speak(60); //Rovers lets out a mighty bark.

    speak.Reset(&felix, &Cat::Meow);
    speak(30); //Felix meows.

    //Callbacks can be set to free functions.
    speak = Func;

    //Copy and assignment operators are well defined.
    ncore::Callback1<void, int> copy = speak;
    ASSERT(copy == speak);

    //Callbacks can be set to null.
    copy.Reset();
    ASSERT(!copy.IsSet());

}