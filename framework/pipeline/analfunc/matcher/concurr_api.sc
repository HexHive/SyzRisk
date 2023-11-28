import $file.proto, proto._

object ConcurrencyAPIMatcher extends Matcher {
  def name = "concurr_api"
  def attr = List()
  def Run(method: Method, 
    version: String,
    GetMetadata: String => List[String]
  ): List[AstNode] = {
    val concurr_gen = method.call("\\w*(wake|_lock|_unlock|thread|_mb|sleep|sync)\\w*").l
    val concurr_irq = method.call("\\w*irq\\w*")
          .containsCallTo("\\w*(enable|disable|restore)\\w*").l
    val concurr_sem_up = method.call("up").l
    val concurr_sem_dn = method.call("down(_[a-z]+)?").l

    (concurr_gen ++ concurr_irq ++ concurr_sem_up ++ concurr_sem_dn)
          .filter(_.tag.name("m").l.nonEmpty).l
  }
}
