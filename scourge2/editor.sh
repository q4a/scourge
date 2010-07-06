JAVA=java

CP=bin:out/artifacts/scourge2/scourge2.jar
for jar in lib/*.jar; do
	CP=$CP:$jar
done
for jar in lib/lib/lwjgl/*.jar; do
	CP=$CP:$jar
done
$JAVA -cp $CP org.scourge.editor.Editor
