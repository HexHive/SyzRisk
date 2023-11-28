import $file.proto, proto._

object InsideGotoMatcher extends Matcher {
  def name = "inside_goto"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    val goto_labels = method.ast.filter(
                  _ match { 
                    case x: JumpTarget => 
                      x.parserTypeName == "CASTLabelStatement"
                    case _ => false
                  }).l

    if (goto_labels.isEmpty) List()
    else { 
      val blocks = goto_labels.isCfgNode.dominates
      (goto_labels ++ blocks).filter(_.tag.name("m").l.nonEmpty).l
    }
  }
}
