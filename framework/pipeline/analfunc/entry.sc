import scala.io.Source
import java.io.File
import java.nio.file.Files
import java.nio.file.Paths
import java.nio.file.StandardCopyOption.REPLACE_EXISTING
import java.io.BufferedWriter
import java.io.FileWriter
import scala.reflect.io.Directory
import java.time.LocalDateTime
import java.time.format.DateTimeFormatter

import $file.matcher.proto, proto._
import $file.matcher.dummy, dummy._
import $file.decl

def GetMetadata(method_name: String, meta_name: String) = {
  val members = cpg.typeDecl(method_name + "__" + meta_name).member.l
  members.map(_.code.substring(1))
}

def Analyze(ver: String, outdir: String, mlist: Array[Matcher]) = {
  // Load source code.
  importCode("./" + ver + ".c")
  val methods = cpg.method.lineNumber(1).l
  val _nr_methods = Source.fromFile("./" + ver + ".c").getLines.size
  if (_nr_methods != methods.length) {
    for (i <- Range(0, _nr_methods)) {
      val filename_str = ".*" + ver + "_" + i.toString + ".c"
      if (cpg.method.filename(filename_str).l.isEmpty) {
        Files.copy(Paths.get("./body/" + ver + "_" + i.toString + ".c"),
            Paths.get("../error_" + DateTimeFormatter.ofPattern("yyMMdd_HHmmss").format(LocalDateTime.now) + ".c"),
            REPLACE_EXISTING)
      }
    }
  }

  // Tag diff lines.
  for (method <- methods) {
    val filename = method.filename.split("/").last
    println(filename)
    val nl_file = Source.fromFile("./line/" + filename)
    for (line <- nl_file.getLines()) 
      method.ast.lineNumber(line.toInt).newTagNode("m").store
    nl_file.close() 
  }
  run.commit

  // Reflect attributes.
  val all_attrs = mlist.flatMap(_.attr).distinct
  for (attr <- all_attrs) {
    attr match {
      case "ossdataflow" => run.ossdataflow
      case _ => throw new RuntimeException(s"unknown matcher attribute ${attr}")
    }
  }

  // Launch matchers.
  for (method <- methods) {
    for (matcher <- mlist) {
      val nodes = matcher.Run(method, ver,
          (s: String) => GetMetadata(method.name, s))
      if (nodes.nonEmpty) {
        val linenos = nodes.flatMap(_.lineNumber).distinct
        println("info: function taken. (name=" + method.name + ", file=" + 
            method.filename.split("/").last + ")") 
        val hexsha = GetMetadata(method.name, "HEXSHA")(0) 
        println(" - hexsha=" + hexsha) 
        val patdir = outdir + "/" + hexsha + "/" + method.name + "/" + matcher.name
        new File(patdir).mkdirs
        val resfile = new File(patdir + "/" + ver)
        val bw = new BufferedWriter(new FileWriter(resfile))
        for (lineno <- linenos) 
          bw.write(Integer.toString(lineno) + '\n')
        bw.close()
      }
    }
  }
}

@main def main(outdir: String, matcher: String) = {
  // Sanity check: check if the 'outdir' exists.
  if (!Files.exists(Paths.get(outdir)))
    throw new RuntimeException("'outdir' not exist")

  // Prepare matchers.
  val mlist_name: Array[String] = 
      if (matcher == "all") { 
        decl.MATCHERS.keys.toArray.filter(_ != DummyMatcher.name) 
      } else if (matcher == "none") { Array() }
      else { matcher.split(" ") }

  val mlist: Array[Matcher] = mlist_name.map(decl.MATCHERS(_))

  // Analyze (i.e., loading - tagging - launching).
  Analyze("new", outdir, mlist)
  workspace.reset()
  Analyze("old", outdir, mlist)
}
