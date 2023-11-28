import $file.proto, proto._

object EnteringGotoMatcher extends Matcher {
  def name = "entering_goto"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    val gotos = method.goto.l
    if (gotos.isEmpty) return List()

    val gotos_postdom = gotos.postDominates.l
    val gotos_ctrl = gotos.astParent.map(
                        _ match { 
                          case x: Block => x.astParent
                          case x => x
                        }).l

    val gotos_all = gotos ++ gotos_postdom ++ gotos_ctrl
    gotos_all.filter(_.tag.name("m").l.nonEmpty).l
  }
}
