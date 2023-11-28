import $file.proto, proto._

object DummyMatcher extends Matcher {
  def name = "dummy"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    println("hi! I'm " + version + ".")
    println(method.name)
    println(GetMetadata("HEXSHA"))
    println(GetMetadata("HEXSHA")(0))
    List(method)
  }
}
