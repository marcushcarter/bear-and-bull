# Bear & Bull (v0.1.15)

## Update Log

- *2025-05-04* started brainstorming, worked on moving cards around
- *2025-05-05* worked on temporary card movement and zones
- *2025-05-06* change zone name to "playzones", made "add_card" function, made "draw_cards" function, added game seeds, made cards bigger when you pick them up, 
- *2025-05-07* added pickup tickets when drawing cards
- *2025-05-08* added card fanning (angle and translation), organized code, cleaned up, changed how cards are rendered
- *2025-05-09* added some base stats for cards, fixed fanning, added deck texture, finished card shmovement prototype, started translating to opengl

- *2025-05-11* switched game idea, seperated window scale to x/y, changed make_zone function, gave up on opengl, draw_card_id func, delete delay, render func rewrite, thunk up the new game idea, brainstorm gameloop
- *2025-05-12* money to draw cards, sell cards for money (and then took it out), redid all of the drawing / moving card func, set limits on where certainc ards can be put, changed some cards' data
- *2025-05-16* added gamestates, pauses, main menu and some other stuff
- *2025-05-17* button hover, new textures, cleaned up, redid render and update funcs again for gamestates, nimations classes with callbacks, decided on a name
- *2025-05-18* some formatting, settings page, asspect ratio fixing, added card present anim, added draft state, added deck state, collection state,
- *2025-05-29* rarities, issellable and rarity overlays, random rarity level table

### NEW idea

- start with a set of 10 common cards in your deck
- start with only being able to draw 1 card at the beggining of rounds (max 6)
- start with only 3 hand slots (max 6)

- each round is 60 seconds
- the goal of each round is to buy/sell stocks to earn as much money as possible
- at the beggining of the round you will draw cards from your deck
- you can draw additional cards from your deck for an cost that increases every draw (resets at begging of the round) (ex. $5 -> $7 -> $9) <- draw price and inflation
- you only getting a maximum amount of cards you can extra draw each round (stat)

- in each round you are given 5 different stocks to trade from 
- each of these stocks charts are previewed in the STOCK CHARTS section, but can be highlighted to view full stats

- after 30 seconds, an event card will appear in the EVENT zone which will greatly positively or negatively affect your round
- you cannot sell any event cards ever and no cards of any type can be put into the EVENT zone
- you can add the event card to your hand for a price and it will be added to your deck at the end of the round
- in the EVENT zone, cards effect are double
    - so if it is a good card, you can leave it in the EVENT zone and risk losing it for double rewards, or you can add it to your deck permanently
    - if it is a bad card, you can add it to your hand to decrease the effect, but you are at risk of getting it in the future from your deck, or you can leave it there and deal with the effects
- if by the end of the round you do not move this card into your hand, it will be destroyed
- you may also sell these cards by using "PERMA CHIPS" which are used every time you sell an unsellable card

- once the round is over, you will be graded on how much money you made, cards sold, cards drawn etc and get bonus money based on your grade
- you will also get to draft out of 3 options consisting of cards, stat increases, money and PERMA CHIPS
- these will either affect your person or be added to your deck
- you can increase how many cards you can draft from a stat boost (from drafting -> RARE)
- stat increases: (number of draw cards, hand slots, draft count, luck, extra draw max, draw price, draw price inflation,)

- You may get loans as teh round goes on but you must pay them back for your run to be validated

## Candlesticks (HOCL)

High (H) - the highest price in that time interval
Open (O) - the frist price in a time interval
Close (C) - The last price in a time interval
Low (L) - the lowest price in that time interval

    |         ← High
   ┌┴┐        ← Open (top of red candle)
   │ │        ← Body (Open to Close)
   └┬┘        ← Close (bottom of red candle)
    |         ← Low

### Other Ideas:

 Guentlets
 Rubber Ducky
 Bananazooka
 Sneeze Gun
 Soda Pop Rocket
 Pop rocks
 Lucky Cahrm Gun
 
 Acorn Slingshot
 Hot Sauce
 Microwave
Pizza Boomerang
 CUpcake explosives
 Crayon
 Ban hammer
 Rolling Pin
 Mace
 Teddy Bear
 Candy Cane

 Ace of Spades
 Rubiks Cube
 Spring Shoes
 Trumpet
 Saxaphone
 Popcorn
 Boom Box
 Water Ballon
 soggy sandwich
 nerf gun
 egg launcher
 money gun
 card launcher
 spoon?
 Yo-Yo
 Soap Gun
 Milk-Up (tiddy version)
 Pogo Stick
 wind up monkey
 toothpaste
 Lawn dart
 Staple gun
 Confetti
 Slinky
 Snowball
 super soaker
 skatebaord
 sticky note
 beez
 foam sword
 teeth
 marshmallow
 cookie
 dice balster Random damage
 ray gun
 black hole?
 RNGesus Can use any ability from your deck
 Pinata
 Eraser
 Golden Goose
 Ticket Vortex
 jester/joke
 sock
 toy gun
 actual gun
 crusty sock?
 glowstick
 big red button
 ktchup
 truck
 chewing gun
 rocket launcher
 tshirt cannon
 cheeze wiz
 dildo
 doritos
 magnet
 gumball
 jawbraker
 finger of fate
 clipboard
 paintbrush
 pine cone
 traffic cone
 tuba (instrument)

 deli cold cut meats
 mood ring

 fish
 copy paste
 grenade
 dungeons and dragons dice 1d20
 glitch stick
 the "nope" button
 fridge
 googly eyes
 dragon princess
 she was a fairy
 tralalero tralala
 tung tung tung tung sahur

 car keys
 ana de armas
 sydney sweeney

 gambling tokens
 spray paint
 taco
 sir top hat
 sundae
 hairbrush
 pretzel
 charlotte merrit
 cat ears
 bras
 fur coat
 dumbell
 birthday present
 pillow fight
 pom poms
 goth mami
 dirty magazine
 nun
 doctor
 christmas elf (santas little helper)
 nurse
 microphone
 
 succubus
 ak-47
 hestia
 zues
 farmer
 hinata
 momo
 maid
 spellbook
 madison sialtsis
 magical wand
 mermaid
 hancuffs/police
 freeze ray
 alexstrasza
 lucia ferrato
 filia skullgirls
 2B / robot
 St Louis
 Isabella Messens
 Alarm Clock
 emoji
 pancake
 pop tart
 eyballs