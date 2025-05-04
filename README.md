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

NOTE: I think im either not doing this right or i just suck at this