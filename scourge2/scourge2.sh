PLATFORM=macosx
JAVA=/System/Library/Frameworks/JavaVM.framework/Versions/1.6.0/Commands/java

CP=out/artifacts/scourge2/scourge2.jar
for jar in lib/*.jar; do
	CP=$CP:$jar
done
for jar in lib/lib/lwjgl/*.jar; do
	CP=$CP:$jar
done
$JAVA -cp $CP -Djava.library.path=./lib/lib/lwjgl/native/$PLATFORM org.scourge.Main
