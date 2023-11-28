import $file.proto, proto._

object ChangingErrMatcher extends Matcher {
  def name = "changing_err"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    val ret_err = method.ast.isReturn.code(".*-E[A-Z]+.*").l
    val assn_err = method.assignments.code(".*-E[A-Z]+.*").l
    if (ret_err.isEmpty && assn_err.isEmpty) return List()

    val ret_err_postdom = ret_err.postDominates.l
    val ret_err_ctrl = ret_err.astParent.map(
                        _ match { 
                          case x: Block => x.astParent
                          case x => x
                        }).l

    val ret_err_all = ret_err ++ assn_err ++ ret_err_postdom ++ ret_err_ctrl
    ret_err_all.filter(_.tag.name("m").l.nonEmpty).l
  }
}
