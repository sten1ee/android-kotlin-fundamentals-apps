package com.hopper.tut.aboutme

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.TextView

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        findViewById<Button>(R.id.done_button).setOnClickListener { addNickname(it) }

    }

    private fun addNickname(doneBtn: View) {
        val nnEdit = findViewById<EditText>(R.id.nickname_edit)
        if (nnEdit.text.isBlank()) {
            if (nnEdit.text.isNotEmpty())
                nnEdit.setText("")
        }
        else {
            nnEdit.visibility = View.GONE

            findViewById<View>(R.id.done_button).visibility = View.GONE

            val nnLabel = findViewById<TextView>(R.id.nickname_text)
            nnLabel.text = nnEdit.text.trim()
            nnLabel.visibility = View.VISIBLE
        }
    }
}
