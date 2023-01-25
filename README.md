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
 - Powerbank
 
## How to play

The usage of this device is divided into different stages:
1. ### Start and players detection
At first you will be welcome by a starting screen that explains the main buttons, you can start playing by pushing the lower button B1.
The Card-Dealer will now make a complete turn on itself to detect the number of players and the respective position, to do so each person should come near to the dealer with his hand ( distance sensor used has some problems with certain types of tissues ).

2. ### Card selection and distribution
Once the 360 degress turn is completed youu will be prompted to card selections screen. Here there will be displayed the number of people that where detected. Furthermore it will be asked to choose how many cards distribute to each person at the beginning of the game, this is done by moving the joystick up or down. Once you have set this you can press the button B1 and the dealer will start giving to each person the selected number of cards.

3. ### Game time
At the end of the first card distribution the dealer will position itself in front of the first player, he can decide whether to ask for a card (Hand in front of distance sensor for ~1 sec) or skip to next person (holding the hand in front of distance sensor until the dealer moves). This can be done by each player when it's his turn.

4. ### End game
The game ends depending on which card game you're playing. In general the game will be ended where there's no cards left on the dealer, or you can decide to stop by pushing button B2 and the structure will return to its inital state

 
## How it works

## Software Design

## State Machine
![Immagine 2023-01-18 103110](https://user-images.githubusercontent.com/113623927/213134840-ae089878-d0a2-4a26-8f02-a190b4f40642.png)

## Concise description of states

## Build and burn the project
In order to properly build this project you want to import the respective libraries.




 
