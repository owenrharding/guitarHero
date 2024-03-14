# guitarHero
Guitar Hero on LED Matrix using ATMega324A

This was a really fun project done for Assignment 2 in CSSE2010.
First time using an AVR Microcontroller, and was pretty skeptical at first, but ended up having a lot of fun with it.

![Guitar Hero](https://github.com/owenrharding/guitarHero/assets/133105825/23fe64f9-07b8-4fec-a283-f3a7e9c88175)
![Guitar Hero Demo](https://github.com/owenrharding/guitarHero/assets/133105825/26da588e-d0b2-483d-9762-5354ce8810b5)

Code Functionality:
- Play notes with Push Buttons
- Play notes with Terminal Input
- Future Notes display 'ghost' note before they start to fall down
- Terminal Game Score
- Game Over
- Manual Mode override - push of a button allows to progress clock ticks manually
- Game Countdown displays before game starts
- Seven-Segment Display displays Game Score
- Game Speed can be toggled between three different modes: slow, normal, fast
- Game clock can be Paused
- Combo Scoring

This was also my first project with C and honestly found it really nice to use.
The majority of my code and logic is on game.c and project.c.

Note: Not all the code in this document is mine. Authors are credited on files that they contributed to, however large or little their contribution. My code spans across the following files:
- game.c
- game.h
- project.c
- display.c
- display.h
- pixel_colour.h
- timer1.c
- timer1.h
- timer2.c
- timer2.h
