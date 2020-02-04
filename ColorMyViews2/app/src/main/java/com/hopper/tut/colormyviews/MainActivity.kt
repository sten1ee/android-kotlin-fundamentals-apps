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


private fun <T: View>forEachEx(viewType: Class<T>, view: View, consume: (T) -> Unit) {
    if (viewType.isInstance(view))
        consume(view as T)

    if (view is ViewGroup) {
        view.children.forEach { childView ->
            forEachEx(viewType, childView, consume)
        }
    }
}

private inline fun <reified T: View> forEach(viewType: Class<T>, view: View, noinline consume: (T) -> Unit) {

    forEachEx(viewType, view, consume)
}
