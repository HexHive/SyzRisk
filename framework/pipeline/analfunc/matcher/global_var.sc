import $file.proto, proto._

object GlobalVarMatcher extends Matcher {
  def name = "global_var"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    val all_ident = method.ast.filter(_.tag.name("m").nonEmpty)
        .isIdentifier.name("(?![A-Z0-9_]+).*").l
    val all_local = method.local.name.l ++ method.parameter.name.l

    all_ident.filter(x => !all_local.contains(x.code) && x != "current").l
  }
}
