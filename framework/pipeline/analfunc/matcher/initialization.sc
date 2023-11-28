import $file.proto, proto._

object InitializationMatcher extends Matcher {
  def name = "initialization"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    // Case 1: presence of any recognizable "init" keywords in the prototype.
    if (method.name.contains("_init") || method.name.contains("init_") ||
        method.name.contains("_create") || method.name.contains("create_") ||
        method.name.contains("_alloc") || method.name.contains("alloc_") ||
        //method.name.contains("_register") || method.name.contains("register_") ||
        //method.name.contains("_setup") || method.name.contains("setup_") ||
        GetMetadata("ATTRS").filter(_.contains("init")).nonEmpty)
      return method.ast.filter(_.tag.name("m").nonEmpty).l

    // Case 2: storing to a variable having gone through an allocation pattern.
    // > Extract allocated pointer variables.
    val assns = method.call("<operator>.assignment")
        .filter(_.argument(1).typ.name(".*\\*").nonEmpty)
        .filter(_.argument(2) match { 
          case x: Call => x.name.length > 1 && x.name(0) != "<"
          case x => false }).l
    val assns_alloc = method.call("<operator>.assignment")
        .filter(_.argument(1).typ.name(".*\\*").nonEmpty)
        .filter(_.argument(2) match { 
          case x: Call => x.name.contains("alloc")
          case x => false }).l
    if (assns.isEmpty && assns_alloc.isEmpty) return List()

    val ctrls = assns.map(x => 
          method.ast.lineNumber(x.lineNumber.getOrElse(-1).asInstanceOf[Int] + 1)
        .isControlStructure.code(".*" + x.argument(1).code + ".*").l)
    // FIXME: this should lead to -ENOMEM or a void return.

    val ret1 = ctrls.map(_.whenTrue.ast.isReturn.l)
    val ret2 = ctrls.map(_.whenTrue.ast.isControlStructure
        .controlStructureType("GOTO").isCfgNode.cfgNext.dominates.l)
    val rets = for ((x, y) <- ret1.zip(ret2)) yield x ++ y

    val _alloc_vars = (for ((a, rs) <- assns.zip(rets)) yield
        if (rs.nonEmpty) List(a.argument(1).code) else List()).flatten
    val _alloc_vars_alloc = for (a <- assns_alloc) yield a.argument(1).code
    val alloc_vars = _alloc_vars ++ _alloc_vars_alloc
    if (alloc_vars.isEmpty) return List()

    // > Find all stores to such variables.
    val mods = method.call("<operator>.assignment.*")
        .filter(_.tag.name("m").nonEmpty).l
    if (mods.isEmpty) return List()

    val init_mods = mods.filter(_.argument(1).ast.isIdentifier.code
          .filter(alloc_vars.contains(_)).nonEmpty).l

    // > Find all call-delegated inits.
    val subinits = method.call(".*(init|INIT|set|write).*")
        .filter(_.argument.l.nonEmpty)
        .filter(_.tag.name("m").nonEmpty).l

    val init_subinits = subinits.filter(_.argument(1).ast.isIdentifier.code
          .filter(alloc_vars.contains(_)).nonEmpty).l

    return init_mods ++ init_subinits
  }
}
