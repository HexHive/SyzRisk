import $file.proto, proto._

object SwitchVarModMatcher extends Matcher {
  def name = "swvar_mod"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    /*
    val assignee = method.cfgNode.isControlStructure
        .controlStructureTypeExact("SWITCH")
        .ast.isCallTo("<operator>.assignment.*")
        .argument(1).ast.isIdentifier.l
    val affector = method.ast.filter(_.tag.name("m").nonEmpty)
        .isIdentifier.l
    return affector.filter(x => assignee.filter(_.code == x.code).nonEmpty).l
    */
    val cases = method.cfgNode.filter(_.isInstanceOf[JumpTarget])
        .code("(case.*|default):").l
    val case_doms = for (c <- cases) yield c.dominates
        .lineNumberNot(c.lineNumber.getOrElse(-1)).l
    val case_idents = for (cd <- case_doms if cd.nonEmpty) yield
        cd.isCallTo("<operator>.assignment.*").argument(1)
        .ast.isIdentifier.dedup.l
    val ccvar = (for (i <- case_idents.flatten) yield 
          if (case_idents.filter(_.map(_.code).contains(i.code)).length >
            case_idents.length * 0.6) Option(i) else None)
        .flatten.dedup.l
    val affector = method.ast.filter(_.tag.name("m").nonEmpty)
        .isIdentifier.l
    return affector.filter(x => ccvar.filter(_.code == x.code).nonEmpty).l
  }
}
