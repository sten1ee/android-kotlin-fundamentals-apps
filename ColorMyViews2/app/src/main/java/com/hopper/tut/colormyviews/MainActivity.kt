package com.hopper.tut.colormyviews

import android.graphics.Color
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.TextView
import android.view.ViewGroup
import android.icu.lang.UCharacter.GraphemeClusterBreak.T
import androidx.core.view.children
import kotlin.reflect.KClass


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setupListeners()
    }

    val colors = arrayOf(Color.DKGRAY, Color.CYAN, Color.YELLOW, Color.MAGENTA, Color.BLUE, Color.LTGRAY)

    lateinit var views: List<View>
    fun setupListeners() {
        val views = mutableListOf<View>()
        //collectAll<TextView>(window.decorView as ViewGroup, views)

        forEach(TextView::class.java, window.decorView) { view ->
            views.add(view)
            view.setOnClickListener {makeColored(it)}
        }

        this.views = views
    }

    /**private inline fun <reified T: View, R> forEach(viewGroup: ViewGroup, noinline consume:  (T) -> R) {
        forEachEx<T, R>(T::class.java, viewGroup, consume)
    }*/


    private fun makeColored(view: View) {
        val viewIdx = view.id - R.id.box_1_text
        view.setBackgroundColor(colors[viewIdx % colors.size])
    }
}



private fun <T: View> allViewsInViewGroup(viewType: KClass<T>, view: View): Sequence<T> = sequence {
    if (viewType.isInstance(view))
        yield (view as T)

    if (view is ViewGroup)
        yieldAll(view.children.flatMap { allViewsInViewGroup(viewType, it) })
}

private inline fun <reified T: View> forEach(view: View) {

    allViewsInViewGroup(T::class, view)
}
