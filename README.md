###  WORK IN PROGRESS 

# Card Dealer
 Card Dealer using MSP432P401R - Embedded Software for the Internet of Things  
 Implementation of an automatic dealer of cards with people recognition.
 
## Project Components
 - MSP432P401R TI
 - Educational Boosterpack
 - 28BYJ-48 Step Motor (ULN2003 Driver)
 - HC-SR04 Distance Sensor
 - Dc Motor
 - Breadboard
 - Lego
 - Cardboard
 
## How to play
 Initially the screen will display a waiting statement, you can start playing by pushing the lower button (B1).  
 The Card-Dealer will now make a complete turn on itself to detect how many people there are and where, to do so each person should come near to the dealer with his hand ( distance sensor used has some problems with certain types of tissues ). At the end of this process you must choose how many cards each person will have at the beginning of the game, this is done by moving the joystick up or down, once you have set this you can press the button B1 and the dealer will start giving to each person the selected number of cards.  
 During the game the dealer will position itself in front of each player, he can decide whether ask for a card () or skip to next person ().  
 The game will go on until the button B2 is pressed (STOP) or the are no cards left.
 
# How it works

## Software Design

## State Machine
![Immagine 2023-01-18 103110](https://user-images.githubusercontent.com/113623927/213134840-ae089878-d0a2-4a26-8f02-a190b4f40642.png)

## Concise description of states
 
