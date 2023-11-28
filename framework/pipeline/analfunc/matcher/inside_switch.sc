import $file.proto, proto._

object InsideSwitchMatcher extends Matcher {
  def name = "inside_switch"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    /*
    val switches = method.ast.isControlStructure
        .controlStructureTypeExact("SWITCH").l
    (switches ++ switches.ast.l).filter(_.tag.name("m").nonEmpty).l
    */
    val cases = method.cfgNode.filter(_.isInstanceOf[JumpTarget])
        .code("(case.*|default):").l
    val cases_body = cases.filter{ x => 
        val len = x.dominates.lineNumberNot(x.lineNumber.getOrElse(-1)).tag.name("m").l.length
        val len2 = x.dominates.lineNumberNot(x.lineNumber.getOrElse(-1)).l.length
        len != 0 && len == len2 }.l
    val cases_label = cases.filter(_.tag.name("m").nonEmpty).l
    return cases_body ++ cases_label
  }
}
