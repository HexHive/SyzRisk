import $file.proto, proto._

object PointerOffsetMatcher extends Matcher {
  def name = "ptr_off"
  def attr = List("ossdataflow")
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    // Check if pointer offsets are influenced by arithmetic.
    // Step 1: identify all pointer equivalents.
    val nptrs = method.ast.isIdentifier
        .filter(x => x.name.contains("ptr") || 
                     x.name.contains("addr") ||
                     x.name.contains("len") ||
                     x.name.contains("off") ||
                     x.name.contains("size") ||
                     x.name.contains("num_"))  
        .isCfgNode.l

    val ptr_eqs = nptrs

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

    (mptr_infs ++ ptr_minfs).dedup.l
  }
}
