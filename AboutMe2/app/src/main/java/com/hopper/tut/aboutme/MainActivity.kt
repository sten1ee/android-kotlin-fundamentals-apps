package com.hopper.tut.aboutme

import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import com.hopper.tut.aboutme.R.string.what_is_your_nickname

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        findViewById<Button>(R.id.done_button).setOnClickListener { done_btn ->
            val nn_edit = findViewById<EditText>(R.id.nickname_edit)
            val nn_label = findViewById<TextView>(R.id.nickname_text)
            if (nn_edit.text.isBlank()) {
                nn_edit.setText(resources.getText(what_is_your_nickname))
                return@setOnClickListener
            }

            nn_label.text = nn_edit.text
            nn_edit.visibility = View.GONE
            done_btn.visibility = View.GONE
            nn_label.visibility = View.VISIBLE
        }
    }
}
