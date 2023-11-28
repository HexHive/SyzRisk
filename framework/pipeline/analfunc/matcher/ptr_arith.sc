import $file.proto, proto._

object PointerArithMatcher extends Matcher {
  def name = "ptr_arith"
  def attr = List("ossdataflow")
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    // Check if pointer equivalents are influenced by arithmetic.
    // Step 1: identify all pointer equivalents.
    val ptrs = method.ast.isIdentifier.filter(_.typ.name(".*\\*").nonEmpty)
        .isCfgNode.l

    val allocs = method.call("\\w*(alloc|_map|map_|mmap)\\w*").argument
        .ast.isIdentifier.isCfgNode.l

    val memops = method.call("(copy_from_user|copy_to_user" +
                             "|memset|memmove|memcpy|memzero.*" +
                             "|readl|writel)").argument
        //.ast.isIdentifier
        .isCfgNode.l

    val subscrs = method.call("<operator>.indirectIndexAccess")
        //.argument(2).ast.isIdentifier // NOTE: Joern ossdataflow + subscr = NOPE
        .isCfgNode.l

    val derefs = method.call("<operator>.assignment.*")
        .filter(_.argument(2).isCallTo("<operator>.indirection").nonEmpty)
        .argument(1).ast.isIdentifier.isCfgNode.l

    val ptr_eqs = ptrs ++ allocs ++ subscrs ++ derefs

    // Step 2: find all variables applied to arithmetic operations.
    val ari_assnas = method.call("<operator>.assignment")
        .filter(x => x.argument(2).ast
            .isCallTo("<operator>.(addition|subtract.*|multipl.*|or|and)").nonEmpty ||
          x.argument(2).ast.isCfgNode.reachableBy(
              method.call("<operator>.(addition|subtract.*|multipl.*|or|and)")).nonEmpty)
        .argument(1).l

    val ari_aassns = method.call("<operator>.assignment(Plus|Minus|Mul).*")
        .argument(1).l

    val ari_unops = method.call("<operator>.(pre|post)(Incr|Decr).*")
        .argument(1).l

    val ari_cmps = method.call("<operator>.(less.*|greater.*|equals|notEquals)")
        .argument.l

    val ari_macros = method.call("<operator>.assignment")
        .filter(x => x.argument(2).isCallTo("[A-Z0-9_]+").nonEmpty)
        .argument(1).l

    val ari_conv = method.call("<operator>.assignment")
        .filter(x => x.argument(2).isCallTo(".*(to_be32|to_le32|be32_to|le32_to|ntoh|hton).*").nonEmpty)
        .argument(1).l

    val aris = ari_assnas ++ ari_aassns /*++ ari_cmps*/ ++ ari_macros ++ ari_unops

    // Step 3: check if any of pointer equivalents are affected by modified 
    // arithmetic operations.
    val mptr_infs = ptr_eqs.filter(_.tag.name("m").nonEmpty)
        .filter(_.reachableBy(aris).nonEmpty).l
    val ptr_minfs = aris.filter(_.tag.name("m").nonEmpty)
        .filter(x => ptr_eqs.reachableBy(x).nonEmpty).l

    // Bonus: arithmetic operation inside an array subscript.
    val ari_subscrs = method.call("<operator>.indirectIndex.*")
        .filter(_.tag.name("m").nonEmpty)
        .filter(_.ast.isCallTo("<operator>.(addition|substract|multipli).*").nonEmpty).l

    (mptr_infs ++ ptr_minfs ++ ari_subscrs).dedup.l
  }
}
