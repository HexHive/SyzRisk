import scala.io.Source

// Load source code.
importCode(@@BODY_PATH@@)
val methods = cpg.method.lineNumber(1).l
assert(methods.length == 1)

// Tag diff lines.
val method = methods(0)
val nl_file = Source.fromFile(@@LINE_PATH@@)
for (line <- nl_file.getLines()) 
  method.ast.lineNumber(line.toInt).newTagNode("m").store
nl_file.close() 
run.commit

