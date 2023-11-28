import $file.proto, proto._

object LockedContextMatcher extends Matcher {
  def name = "locked_ctx"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    if (method.name.takeRight(5) == "_lock" ||
        method.name.takeRight(7) == "_locked")
      return method.ast.filter(_.tag.name("m").nonEmpty).l

    // Case 1: when a function is annotated with __must_hold
    if (GetMetadata("ATTRS").filter(_.contains("must_hold")).nonEmpty)
      return method.ast.filter(_.tag.name("m").nonEmpty).l

    // Case 2: when parts of a function are lock-protected.
    // - Sub-case 1: when there is a pair of locks.
    val lock_dom = method.call(".*_(lock|acquire).*").dominates.l
    val unlock_postdom = method.call(".*_(unlock|release).*").dominatedBy.l
    val locked_ctx = lock_dom.filter(unlock_postdom.contains).l

    // - Sub-case 2: when a function exits while holding a lock.
    val locked_out = method.call(".*_lock.*")
        .filter(_.dominates.isCallTo(".*_unlock.*").isEmpty)
        .dominates.dedup.l

    // - Sub-case 3: when a function releases a lock held on the entry.
    val locked_in = method.call(".*_unlock.*")
        .filter(_.dominatedBy.isCallTo(".*_lock.*").isEmpty)
        .postDominates.dedup.l

    (locked_ctx ++ locked_out ++ locked_in).filter(_.tag.name("m").nonEmpty).l
  }
}
