import $file.proto, proto._

object PointerAPIMatcher extends Matcher {
  def name = "ptr_api"
  def attr = List("ossdataflow")
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    // Check if it modifies alreadly known pointer arithmetic APIs.
    val apis = method.call("(pkb|nla)_(put|get|data).*")
        .filter(_.tag.name("m").nonEmpty).l

    val memops = method.call("(copy_from_user|copy_to_user" +
                             "|memset|memmove|memzero.*" +
                             "|memcpy|readl|writel|.*_copy_.*)")
        .filter(_.tag.name("m").nonEmpty).l

    (apis ++ memops).dedup.l
  }
}
