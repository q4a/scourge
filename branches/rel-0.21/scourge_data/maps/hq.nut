// script loaded when in HQ map
function startLevel() {
//  foreach(member,val in getroottable()){
//    print(member+"\n");
//  }
  print( "Welcome to S.C.O.U.R.G.E.: Heroes of Lesser Renown\n" );
  print( "v" + ::scourgeGame.getVersion() + "\n" );
  print( "You are in the head-quarters level.\n" );
}

function endLevel() {
  i<-0;
  for( i = 0; i < 10; i++ ) {
    print( "i=" + i + "\n" );
  }
  print( "Ending level.\n" );
}

