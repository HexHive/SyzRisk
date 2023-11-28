@main def main() {
  importCode("new.c")
  val methods = cpg.method.lineNumber(1).l
  for (method <- methods) {
    val members = cpg.typeDecl(method.name + "__HEXSHA").member.l
    if (members.isEmpty) {
      println("info: '" + method.name + "__HEXSHA' is missing. (file=" +
              method.filename.split("/").last + ")")
    }
  }
  workspace.reset
  importCode("old.c")
  val methods_old = cpg.method.lineNumber(1).l
  for (method <- methods_old) {
    val members = cpg.typeDecl(method.name + "__HEXSHA").member.l
    if (members.isEmpty) {
      println("info: '" + method.name + "__HEXSHA' is missing. (file=" +
              method.filename.split("/").last + ")")
    }
  }
}
