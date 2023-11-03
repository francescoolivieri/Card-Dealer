# Card Dealer
 Automated Card Dealer using MSP432P401R - Embedded Software for the Internet of Things  
 Implementation of an automatic dealer of cards with people recognition.
 
## Project Components
 - MSP432P401R TI
 - Educational Boosterpack
 - 28BYJ-48 Step Motor (ULN2003 Driver)
 - HC-SR04 Distance Sensor
 - Dc Motor (L298N Driver)
 - Breadboard
 - Lego
 - Cardboard
 - Powerbank

## Links
video: https://youtu.be/4cU7cmrplUQ  
powerpoint: https://docs.google.com/presentation/d/1i0G2VViDwSeIPXIIWysBmuo2hVDScN2D/edit?usp=sharing&ouid=106265802200676387458&rtpof=true&sd=true

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
The game ends depending on which card game you're playing. In general the game will be ended when there's no cards left on the dealer, or you can decide to stop by pushing button B2 and the structure will return to its inital state


## Project layout

```
├── README.md  
├── CardDealer   
│  ├── Binaries		# Folder with executable file    
│  ├── Includes           
│  ├── FreeRTOS		# FreeRTOS functions    
│  ├── LcdDriver	#Libraries for LCD screen    
│  ├── Libraries    
│  	├── DefaultLib     
│  	├── screen
│  	├── inizialization  
│  	├── events  
│  	├── peripherals  
│  	└── actions   
│  	     
└── main_freertos.c	# main with FSM states 
    
```


### Libraries folder
Here we find DefaultLib and all the source files with the functions used in our project:

	- screen.h
	functions to print on the LCD screen the funtions phases

	- initialization.h
	functions to inizialize the hardware end interrupt handlers

	- events.h
	funtions to specify the event happening

	- peripherals.h 
	low level funtions to do basic operations with peripherials (sensors and motors)

	- actions.h
	high level functions to implement the more complex game phases

## State Machine
![Immagine 2023-01-18 103110](https://user-images.githubusercontent.com/113623927/213134840-ae089878-d0a2-4a26-8f02-a190b4f40642.png)

### Concise description of states

- IDLE: Here all periferials are initialized
- WAITING: Here is displayed the functionalities of the buttons and can select B1 to continue with game. From here on you can exit every state by pressing B2 button
- SCANNING: Here the distnce sensor scans end counts player. The stepper motor turns and when encountered a player its position is saved
- GAME SELECTION: state to select the number of card to distribute
- GAME: in game state. If asked cards are deliviered else the stepper motor moves to the next person positions
- STOP EMERGENCY: In every moment the button B2 can be pressed end the machine goes in this state. it then transitions to IDLE.


## Build and burn the project
In order to properly build this project you want to :

(1) Open the project with CCS
	
(2) Right click on "CardDealer [Active - Debug]" folder and left click on "properties"

(3) The "Properties for CardDealer" window will open. Now go to Build->Arm Compiler->Include Options. In the section "Add dir to #include ..." you must change all paths to include the same files with respect to the position where you placed the project. You also need to include the path for "simplelink_msp432p4_sdk_3_40_01_02\simplelink_msp432p4_sdk_3_40_01_02\source" (folder given by professor)

(4) Once the paths are changed go to Build->Arm Linker->File Search Path. Here you need to change paths in the section "Include library file or ..." to include the listed files.

(5) When all done in the bottom right corner click "Apply and close" to apply all changes.

(5) Now you can build the project by clicking the hammer icon on the top bar.

(6) To burn the project on the board connect the MSP432 to your computer with its USB cable and from the top section select Run->Load and select this project.


## Team members work

- Francesco: distance sensor, FSM, button interactions, timers, interrupts, base frame.
- Federico: DC Motor, Lcd display layouts and screens functions, FSM, handlers, wiring.
- Eddie: Joystick interaction, FSM, events, wiring and structure building, stepper motor.



 
