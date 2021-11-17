var EMIXXA = {};
var deckConnection = {};
EMIXXA.prevTime = [0, 0, 0, 0];

/* DECK INITIALIZATION */
EMIXXA.init = function() {

    // Track Position LEDs for Jog Wheel and Slicer
    deckConnection[0]=engine.makeConnection("[Channel1]", "playposition", EMIXXA.TrackPosition);
    deckConnection[1]=engine.makeConnection("[Channel2]", "playposition", EMIXXA.TrackPosition);
}


// Spinning Platter LEDs & Slicer Loop PAD LEDs as beat counter
// pulled together for the calculation to be done only once and then
// send LED signals to Jog or PAD LEDs when needed.
EMIXXA.TrackPosition = function(value, group) {
    // do nothing before track starts
    if (value === 0) {
        print("RESET?!");
        EMIXXA.prevPadLED[deckOffset]=0;
        return;
    }
    // lets define some variables first
    var deckNumber = script.deckFromGroup(group);
    var deckOffset = deckNumber - 1;
    var trackDuration = engine.getValue(group, "duration"); // in seconds
    var playPosition = value * trackDuration; // in seconds
   
    playPosition=trackDuration-playPosition;
    playPosition=Math.floor(playPosition*10);
    //time changed?
    if (playPosition != EMIXXA.prevTime[deckOffset]) {
      print(parseInt((EMIXXA.prevTime[deckOffset]/10)%60));
      var millis = parseInt((playPosition%10))
      , secs = parseInt((playPosition/10)%60)
      , mins = parseInt((playPosition/(10*60))%60);
      print(secs);
      if(parseInt((EMIXXA.prevTime[deckOffset]/(10*60))%60) != mins){
         midi.sendShortMsg(0x94 + deckOffset, 0x14, mins);
      }
      if(parseInt((EMIXXA.prevTime[deckOffset]/10)%60) != secs){
         midi.sendShortMsg(0x94 + deckOffset, 0x15, secs);
      }
      //print(playPosition);
      midi.sendShortMsg(0x94 + deckOffset, 0x16, millis);
      //store time
      EMIXXA.prevTime[deckOffset] = playPosition;
    }
}

EMIXXA.shutdown = function() {
}