package com.hopper.tut.colormyviews

import android.graphics.Color
import android.graphics.drawable.ColorDrawable
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.TextView
import android.view.ViewGroup
import androidx.core.view.children


class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        setupListeners()
    }

    val colors: Array<Int> = arrayOf(
        Color.DKGRAY, Color.CYAN,
        Color.YELLOW, Color.MAGENTA,
        Color.rgb(0xff, 0x7f, 0x50), Color.GREEN, Color.BLUE)

    lateinit var views: List<View>
    val view2colorMap = HashMap<View, Int>()

    fun setupListeners() {
        val views = mutableListOf<View>()

        var count = 0
        allViewsInViewGroup(window.decorView)
            .filter {
                it is TextView || it.id == R.id.top_layout
            }
            .forEach { view ->
                view2colorMap[view] = colors[count++ % colors.size]
                views.add(view)
                view.setOnClickListener { makeColored(it) }
            }

        this.views = views
    }


    private fun makeColored(view: View) {
        //val viewIdx = view.id - R.id.box_1_text
        val oldColor = view.background?.let { (it as ColorDrawable).color }
        view.setBackgroundColor(view2colorMap[view] ?: Color.GREEN) // colors[viewIdx % colors.size])
        oldColor?.let { view2colorMap[view] = it }
    }
}


private fun allViewsInViewGroup(view: View): Sequence<View> = sequence {
    yield(view)

    if (view is ViewGroup)
        yieldAll(view.children.flatMap { allViewsInViewGroup(it) })
}


private inline fun <reified T: View> forEach(view: View, noinline consume: (T) -> Unit) =
    allViewsInViewGroup(view)
        .filterIsInstance<T>()
        .forEach(consume)


private inline fun <reified T: View> all(view: View) =
    allViewsInViewGroup(view)
        .filterIsInstance<T>()
