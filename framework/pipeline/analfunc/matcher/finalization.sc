import $file.proto, proto._

object FinalizationMatcher extends Matcher {
  def name = "finalization"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    if (GetMetadata("ATTRS").filter(_.contains("exit")).nonEmpty)
      return method.ast.filter(_.tag.name("m").nonEmpty).l

    // Check 1: absence of any recognizable allocation call.
    val alloc_calls = method.call("\\w*(alloc|create|add|init|_map)\\w*").l
    if (alloc_calls.nonEmpty) return List()

    // Check 2: absence of any recognizable allocation pattern.
    val assns = method.call("<operator>.assignment")
        .filter(_.argument(1).typ.name(".*\\*").nonEmpty)
        .filter(_.argument(2) match { 
          case x: Call => x.name.length > 1 && x.name(0) != "<"
          case x => false }).l

    val ctrls = assns.flatMap(x => 
          method.ast.lineNumber(x.lineNumber.getOrElse(-1).asInstanceOf[Int] + 1)
        .isControlStructure.code(".*" + x.argument(1).code + ".*").l)

    val ret1 = ctrls.whenTrue.ast.isReturn.l
    val ret2 = ctrls.whenTrue.ast.isControlStructure
        .controlStructureType("GOTO").isCfgNode.cfgNext.dominates.l
    if (ret1.nonEmpty || ret2.nonEmpty) return List()

    // Check 3: presence of recognizable deallocation calls.
    val dealloc_calls = method.call("\\w*(free|remove|delete|del_|unmap|stop|detach|destroy)\\w*").l
    if (dealloc_calls.isEmpty) return List()

    return method.ast.filter(_.tag.name("m").nonEmpty).l
  }
}
