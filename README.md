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
- *2025-05-17* button hover, new textures, cleaned up, redid render and update funcs again for gamestates, nimations classes with callbacks




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

CARD IDEAS

- cash cards that you can sell for money
- loan cards that you can sell for -money




FACTORS THAT AFFECT A STOCKS RISE AND FALL

- supply and demand (higher number means more supply) (events, people can mass buy(higher price)/sell(lower price) stocks)
- earning reports, scandals etc (measured through performance (higher performance means more expensive stock))
- investments, inflation and employment data
- follow crowd behavior
- global events
- taxes
- 

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


















# Retro FPS Deckbuilder (Haven't decided on a game name yet)

Started working on this game on *2025-05-04* (May the force be with you :)

## Opening Ideas / Thoughts

*2025-05-04*

I had this idea of making a retor deckbuiulder fps shooter game. basically you run around completing an objective, searching for relics, and collecting cards that you can equip or use as power ups. In the pause menu (which doesnt actually pause the game) there are four main sections, the deck, the hand, the equipement slots, and the discard zone. Your deck starts out empty when you start the game, and you must collect cards in the game to fill your deck. When you have cards in your deck, you cannot yet draw those cards as you need "pickup tickets" (which are relatively easy to find). You can either use a ticket to draw a card from your deck into your hand, or ue multiuiple tickets to buy cards from the shops (which goes into your deck). Your hand can only have a mximum amount of cards (which can be upgraded) and if you have reached the limit, you cannot draw any cards. 

Once these cards are in your hand, you can move them to either your weapon slot, accesory lot, discard zone or play zone. When a card is in the weapon slot, it uses the weapon attribute of the card, if it is in the accesory slot, it uses the accesory stats of the card. You can also delete a card, which permanently removes it from your hand/deck/equip slot. And you can play a card, which uses the play stats of the card. Each card has individual weappon, accesory and play stats of the card. Cards also have hand holding stats which are only active if that card is in your hand. [so four different stats for holding it in your hand, playing it (usually permanent), equiping it as a weapon, and equiping it as an accessory]

I also had the idea of in the actaul game ui, becasue the game is based off of cards, most of the ui is card based. In the menus, the cursor you use is a little rotated spade icon (which can be costomized). Most of the ui are setup to look like cards. In the actuall game, the reticle looks like a spade (also customizable). The health bar, is a empty heart in the top left corner of the screen, that is filled up when you have full health and starts to leak red particles when you get hurt, start to die. In the bottom left corner of the screen is a hand that is holding a deck of cards (look of deck is customizable) and when you pause, it expands and almost looks like it opens up. In the bottom right corner of the game screen is the visual for whatever weapon you have equiped. And also in the top left are mini icons that show which cards you have equiped. When you find a card in the game world, it enlarges up in te middle of the screen, spins and then lerps to the hand holding the deck in the bottom left corner.

I first want to kind of design the layout of the "pause menu" (but not really pausing). Im going to use just sdl to design the layout of the pause menu and card movement becasue im not too comfotable with OpenGL yet. So i have to make hitboxes (or at least the visuals) for:

- the deck on the bottom left of the menu (just a visual) (number showing how many cards are in your deck)
- the weapon equip slot (maximum 1 cards) (ex. <code>1/1</code> shows if the slot is filled)
- the accessory equip slot (maximum 1 cards) (ex. <code>1/1</code> shows if the slot is filled)
- the cursor (customizable texture)
- the players hand zone (maximum <code>num hand slots</code> cards) (ex. <code>3/5</code> shows how many hand slots are filled)
- the play zone
- the delete zone
- place for your pickup tickets to be shown

So lets get started I guess :|

First I wanted to make the window resizable but always keep the same aspect ratio. (it didnt work so for now the window will stay at 1920x1280 scaled down to 1500x1000). Im going to use % of the screen to render these boxes because thats how it works in OpenGL (window_width * 0.1). I did this by multiplying the normal x/y positions at 1500x1000 pixel window (which is the default) and multiply it by the current screen width (in pixels) divided by 1500. Then I setup a deck, a hand zone, discard pile and a equipement zone.

**NOTE: I think im either not doing this right or i just suck at this**

*2025-05-05*

Now I need to actually give each "card" some data that tells what card it actually is / what it does. Im thinking about doing this with either a strcut of these inside the SoA or even just adding more lists into the SoA for the card stats. I think that is what i will do as the structs in the SoA take alot of memory and processing time. I did this by assigning a card a specific ID, which is linked to a seperate SoA that has all they data, so from just the cards ID, you can find all its data. Pretty much the "Cards" SoA is for the position of the cards and the CardID SoA is the data

### ***WTF did I do? (i cant unbreaked the deck picker upper)*** I fixed it

Then i tried to add card fanning to the rendered cards, i di this by finding the average of the cards which is either the middle of a card for an odd amount or the average position between the two for an even number. It then takes the card number in the zoneminus the total number of cards that can fit in the zone divide dby 2. You multiply that by a angle magnitude (I used 10) Nd thats what you change the render angle by.



**Each Card has stats for a bunch of attributes:**

Weapon
- type (melee, hitscan, none, physics projectiles)
- range (hitscan weap)
- damage
- damage multiplier
- effect (burn, freeze, none)
- effect chance (%)
- life steal (% of health)
- collateral (%)


***Card naming***
- int ID
- string name
- stringe description
***For in game weapons attributes***
- range (m)
- 
- 
- 
- 
- 
- 
- 
- 

***And for when youre holding it gives a stat boost***
- hp increase (int)
- life steal chance (%)
- pickup range (int)
- ammo capacity increase (int)
- armor increase (%)
- speed increase (int)
- 

## Weapon Cards

*2025-05-04*

### 8-Balls (RARE)
A small blueish-black orb with a brighter blue screen on it. When you use it, it picks a random damage multiplier from 0.1x to 10x and that is the damage it will do if it hits an enemy. This is a medium ranged weapon with medium attack speed and somewhat fast movement speed. Just a normla enmpty hand when thrown. Reaches off screen for a new ball afterwards. 

### Fireball (RARE)
A glowing orange ball of fire that can set enemies on fire. When you use it, it can bounce off a surface once and then will explode on the second (or when it hits an enemy), which does damage, and also may give the enemy the burn effect. THis ball of flame floats in your hand until you choose to throw it. Then will it is cooling down, it will slowly grow bigger in your hand. This is a medium ranged weapon with a slow cooldown, but high damage 

### Fists (COMMON)
You just punch enemies with your bare hands. They always said your hands should be registered as weapons. You just punch your enemies dum dum. A melee weapon with very fast movement speed, fast attack speed and medium damage.

### Finger Guns (UNCOMMON)
Since when did you have bullets coming out of your fingers. When you use this weapon, you will shoot fingernails out at your enemies. This is a medium ranged weapon with medium cooldown, medium damage and decent cooldown speed and somewhat fast movement speed. This gun can only hold 5 fingernails at a time, then you have to reload. Just put your hand in a gun shape and fire (which makes a flash of light at the fingertip) and reloads like a pump shotgun.

### Coin Toss (RARE)
You flip a coin that can land on heads or tails. On heads, this will kill the nearest enemy to your location, if you land on tails, the coin will attack you and take 20% of your health. This is a special weapon with unlimited damage, no reload and high movement speed. It is a risk reward type of card. When attacking, the player will flip the coin in thier hand.

### Pineapple (UNCOMMON)
A prickly yellow pineapple. It is a short ranged weapon thatdoes very little damage but has a very high change of inflicting the impaled / bleeding effects. This pinapple just sits in your hand and you throw it. You get a new one off screen.

### Traffic Cone (COMMON)
A bright orange traffic cone. You just attack normally with it. A melee weapon with aomewhat fast move speed, higher damage than the fists and somewhat fast attack speed. You attack people with a orange traffic cone.

### Deck Shuffler (LEGENDARY)
Holds a card spread in your hand. Card looks like a deck of cards. When you use this weapon, it will pick a random card from your deck and use it as ammo. If you have no cards, you cant attack. These cards have unlimited range (until they hit a wall or enemy) and do a massive amount of damage. When used, it just looks like you throw a card ahead of you. Reload by grabbing a new card from your deck on the left side of the screen. Also has fast movement speed and decent reload time.

### Magic Wand (RARE)
A wooden magical wand that can cast spells. When you attack with this weapon, you will cast a random spell from the types of: fireball, ice, bubble, lightning, bleed (thorn) etc which has a medium chance to effect thos effects. This attack is a mid range attack and has faster movement speed, no reload, unlimited uses and minimal damage. The casting frame looks like the Avada Cadavra spell motion.

### Hot Suace Pistol (RARE)
A hot sauce bottle with a firing hammer and a trigger. Card looks like the outside of a hot sauce bottle. When attacking, this weapon has a chance to burn the enemy and also cuases higher damage. THis weapon is medium range and has medium move speed, slower reload and higher damage. You have to reload after every shot. Reload animation looks like a little injector you squirt hot sauce back into the bottle.

### Toast (COMMON)
Literally a piece of toast. Card looks like a piece of bread. This piece of toast slowly heals you over time. It is a melee weapon that does very small amount of damage, moves very very fast. animation just looks like youre hitting people with a piece of toast

### FRENCH Toast (UNCOMMON)
A piece of FRENCH Toast. Card looks like a piece of frnech toast. This french toast heals you over time. It is a melee weapon that does a bit more than very small amount of damage, moves very very fast. Animation just looks like you are hitting people with some soggy toast.

## Playable Cards

*2025-05-04*

### CTRL + ALT + DELETE (EPIC)
A little keyboard with only 3 keys (ctrl, alt, delete). When used, it kills all enemies within a certain radius but doesn't drop the items they may give you. When the card is used, a small explosion circle happens on the screen and the enemies all evaporate.

### (Princess) Peach Juice (UNCOMMON)
A juice label showing princess peach holding a bottle with the image of a peach on it. When used, you will gain some speed but lose some stamina. When used, the player looks like they take a sip of the drink and then the bottle is thrown as a projectile.

### Fake ID (UNCOMMON)
A little fake id card with the image of an ugly girl (just a normal id layout). Using it will raise your defense stat. When you use this item, it will look like you are showing someone in front of you your ID, pinting it away from the camera.

### Lemon-Aide (RARE)
A little juice box of lemonade. Using it will increase you hp and increase your maximum hp as well. Using it looks like squeezing the juice box and particles of lemonade squirting out everywhere.

### Running Shoes (COMMON)
A pair of limited edition running shoes. When used, they shoes increase your speed. The shoes will just move out of the bottom of the screen when they are equiped.

### African Prince (EPIC)
A little phone with the email from an African Prince. So remember that African Prince that wanted to gie you majority of his fortune? Yeah hes back. You imediately get a large number of Pickup Tickets. WHen you use it, you will cal the phone and a bunch of ticket particles will fall from the sky. 

### Mini V8 (LENEGDARY)
A mini v8 engine block. When you use this item, it will give you a absolutely massive boost in speed and massive boost in stamina. When you use this item, it will flash blue, sdissapear and a "vroom vroom" sound will play.

### Aphrodite's Kiss (Super Gioffy) (RARE)
A card with a picture of aphrodite blowing a kiss with a little kiss particls. When used, it will give you a small boost in all stats (speed, strength, defense) and a larger increase in stamina (becasue aphrodite loves stamina). When you use this item, a bunch of kiss particles will spawn and circle around you and a kissing sound will play.

# I identify as he/he















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