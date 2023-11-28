// Matcher files
import $file.matcher.proto, proto._
import $file.matcher.dummy, dummy._
import $file.matcher.inside_goto, inside_goto._
import $file.matcher.entering_goto, entering_goto._
import $file.matcher.finalization, finalization._
import $file.matcher.concurr_api, concurr_api._
import $file.matcher.ptr_arith, ptr_arith._
import $file.matcher.ptr_api, ptr_api._
import $file.matcher.ptr_off, ptr_off._
import $file.matcher.initialization, initialization._
import $file.matcher.mm_api, mm_api._
import $file.matcher.global_var, global_var._
import $file.matcher.locked_ctx, locked_ctx._
import $file.matcher.swvar_mod, swvar_mod._
import $file.matcher.inside_switch, inside_switch._
import $file.matcher.changing_err, changing_err._

// Registration
val MATCHERS: Map[String, Matcher] = Map(
  /* --- */
  InsideGotoMatcher.name    -> InsideGotoMatcher,
  EnteringGotoMatcher.name  -> EnteringGotoMatcher,
  FinalizationMatcher.name  -> FinalizationMatcher,
  ConcurrencyAPIMatcher.name-> ConcurrencyAPIMatcher,
  PointerArithMatcher.name  -> PointerArithMatcher,
  PointerAPIMatcher.name    -> PointerAPIMatcher,
  PointerOffsetMatcher.name -> PointerOffsetMatcher,
  InitializationMatcher.name-> InitializationMatcher,
  MemoryMgmtAPIMatcher.name -> MemoryMgmtAPIMatcher,
  GlobalVarMatcher.name     -> GlobalVarMatcher,  
  LockedContextMatcher.name -> LockedContextMatcher,  
  SwitchVarModMatcher.name  -> SwitchVarModMatcher,  
  InsideSwitchMatcher.name  -> InsideSwitchMatcher,  
  ChangingErrMatcher.name   -> ChangingErrMatcher,
  /* --- */
  DummyMatcher.name         -> DummyMatcher
)
