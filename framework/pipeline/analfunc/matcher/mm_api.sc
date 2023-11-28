import $file.proto, proto._

object MemoryMgmtAPIMatcher extends Matcher {
  def name = "mm_api"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    val mm_low = method.call("\\w*(alloc|free)\\w*").l
    val mm_ref = method.call("\\w+(_get|_put)").l

    (mm_low ++ mm_ref).filter(_.tag.name("m").l.nonEmpty).l
  }
}
