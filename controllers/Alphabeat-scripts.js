var ALPHABEAT = {};
ALPHABEAT.prevTime = [0, 0, 0, 0];

/* DECK INITIALIZATION */
ALPHABEAT.init = function() {

    // Track Position LEDs for Jog Wheel and Slicer
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

ALPHABEAT.shutdown = function() {
}