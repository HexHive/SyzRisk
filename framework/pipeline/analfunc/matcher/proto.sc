abstract class Matcher {
  def name: String
  def attr: List[String]
  def Run(method: Method, version: String,
      GetMetadata: String => List[String]): List[AstNode]
}
