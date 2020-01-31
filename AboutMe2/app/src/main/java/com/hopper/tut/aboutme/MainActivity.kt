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

        findViewById<Button>(R.id.done_button).setOnClickListener { doneBtnClicked(it as Button) }

    }

    private fun doneBtnClicked(doneBtn: Button) {
        val nnEdit = findViewById<EditText>(R.id.nickname_edit)
        val nnLabel = findViewById<TextView>(R.id.nickname_text)
        if (nnEdit.text.isBlank()) {
            if (nnEdit.text.isNotEmpty())
                nnEdit.setText("")
        }
        else {
            nnLabel.text = nnEdit.text.trim()
            nnEdit.visibility = View.GONE
            doneBtn.visibility = View.GONE
            nnLabel.visibility = View.VISIBLE
        }
    }
}
