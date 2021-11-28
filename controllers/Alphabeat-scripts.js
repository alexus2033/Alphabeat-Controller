var ALPHABEAT = {};
ALPHABEAT.prevTime = [-1, -1, -1, -1];

/* DECK INITIALIZATION */
ALPHABEAT.init = function() {

    // update Displays
    engine.makeConnection("[Channel1]", "playposition", ALPHABEAT.TrackPosition);
    engine.makeConnection("[Channel2]", "playposition", ALPHABEAT.TrackPosition);
}


ALPHABEAT.TrackPosition = function(value, group) {
    // do nothing before track starts
    if (value === 0) {
        ALPHABEAT.prevPadLED[deckOffset]=0;
        return;
    }
    // define some variables first
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    var trackDuration = engine.getValue(group, "duration"); // in seconds
    var playPosition = value * trackDuration; // in seconds
   
    playPosition=trackDuration-playPosition;
    playPosition=Math.floor(playPosition*10);
    //time changed?
    if (playPosition != ALPHABEAT.prevTime[deckOffset]) {
      print(parseInt((ALPHABEAT.prevTime[deckOffset]/10)%60));
      var millis = parseInt((playPosition%10))
      , secs = parseInt((playPosition/10)%60)
      , mins = parseInt((playPosition/(10*60))%60);
      print(secs);
      if(parseInt((ALPHABEAT.prevTime[deckOffset]/(10*60))%60) != mins){
         midi.sendShortMsg(0x94 + deckOffset, 0x14, mins);
      }
      if(parseInt((ALPHABEAT.prevTime[deckOffset]/10)%60) != secs){
         midi.sendShortMsg(0x94 + deckOffset, 0x15, secs);
      }
      //print(playPosition);
      midi.sendShortMsg(0x94 + deckOffset, 0x16, millis);
      //store time
      ALPHABEAT.prevTime[deckOffset] = playPosition;
    }
}

ALPHABEAT.brake_button = function(channel, control, value, status, group) {
      var deck = parseInt(group.substring(8,9)); // work out which deck we are using 
      var activate = value > 0;
      
      if (activate) {
          engine.brake(deck, true); // enable brake effect
      } else {
          engine.brake(deck, false); // disable brake effect
      }
}

ALPHABEAT.shutdown = function() {
    //reset display
    midi.sendShortMsg(0x90, 0x3F, 0x7F);
}