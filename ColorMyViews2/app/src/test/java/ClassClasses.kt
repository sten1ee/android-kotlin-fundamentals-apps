import kotlin.reflect.KClass

inline fun <reified T> callWithReifiedJavaClass() = callWithExplicitJavaClass(T::class.java)
fun <T> callWithExplicitJavaClass(clasz: Class<T>) = println(clasz)

inline fun <reified T: Any> callWithReifiedKClass() = callWithExplicitKClass(T::class)
fun <T: Any> callWithExplicitKClass(clasz: KClass<T>) = println(clasz)

inline fun <E, reified T: Collection<E>> callWithReifiedJavaClassAndInstance(t: T) = callWithExplicitJavaClassAndInstance(T::class.java, t)
fun <T> callWithExplicitJavaClassAndInstance(clasz: Class<T>, t: T) = println(t)

inline fun <E, reified T: Collection<E>> callWithReifiedKClassAndInstance(t: T) = callWithExplicitKClassAndInstance(T::class, t)
fun <T: Any> callWithExplicitKClassAndInstance(clasz: KClass<T>, t: T) = println(t)

fun main() {
    callWithReifiedJavaClass<Sequence<Int>>()
    callWithReifiedKClass<Sequence<Float>>()

    callWithReifiedJavaClassAndInstance(setOf(1, 2, 3))
    callWithReifiedKClassAndInstance(listOf(1.1, 2.4, 3.6))
}
