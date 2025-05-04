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

First I wanted to make the window resizable but always keep the same aspect ratio. (it didnt work so for now the window will stay at 1920x1280 scaled down to 1500x1000). Im going to use % of the screen to render these boxes because thats how it works in OpenGL (window_width * 0.1)

**NOTE: I think im either not doing this right or i just suck at this**

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
A bright orang traffic cone. You just attack normally with it. A melee weapon with aomewhat fast move speed, higher damage than the fists and somewhat fast attack speed. You attack people with a orange traffic cone.

### Deck Shuffler (LEGENDARY)
Holds a card spread in your hand. Card looks like a deck of cards. When you use this weapon, it will pick a random card from your deck and use it as ammo. If you have no cards, you cant attack. These cards have unlimited range (until they hit a wall or enemy) and do a massive amount of damage. When used, it just looks like you throw a card ahead of you. Reload by grabbing a new card from your deck on the left side of the screen. Also has fast movement speed and decent reload time.

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

# I identify as he/he

















- Guentlets
- Rubber Ducky
 Bananazooka
 Sneeze Gun
 Soda Pop Rocket
 Pop rocks
 Lucky Cahrm Gun
 Toast
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
 8 -ball
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
 greek goddess super gioffy
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